const sql = require('mssql');
const WebSocket = require('ws'); // Asegúrate de importar WebSocket

// Configuración de la conexión a la base de datos
const config = {
    user: 'hotel@dbhotel-a',
    password: 'Proyecto@1',
    server: 'dbhotel-a.database.windows.net',
    database: 'DBHotel',
    port: 1433,
    options: {
        encrypt: true,
        trustServerCertificate: true,
    },
    pool: {
        max: 10,
        min: 0,
        idleTimeoutMillis: 30000,
    },
};

// Función para conectarse a la base de datos
async function connectToDatabase() {
    try {
        console.log('Attempting to connect to the database...');
        await sql.connect(config);
        console.log('Connected to the database');
    } catch (err) {
        console.error('Error connecting to the database:', err);
        setTimeout(connectToDatabase, 2000);
    }
}

// Función para obtener todos los registros de la tabla Registro
async function getAllRegistros() {
    try {
        let pool = await sql.connect(config);
        let result = await pool.request().query('SELECT * FROM Registro');
        return result.recordset;
    } catch (err) {
        console.error('Error al obtener los registros:', err);
        throw err;
    }
}

// Función para verificar cambios en la tabla Registro
async function checkForChanges(wss) {
    let previousRecords = [];

    setInterval(async () => {
        try {
            const currentRecords = await getAllRegistros();

            // Comparar los registros actuales con los anteriores
            let recordsToUpdate = currentRecords.filter((currentRecord) => {
                const previousRecord = previousRecords.find(record => record.uuid === currentRecord.uuid);
                return !previousRecord || JSON.stringify(previousRecord) !== JSON.stringify(currentRecord);
            });

            if (recordsToUpdate.length > 0) {
                console.log('Nuevos cambios detectados en la base de datos:', recordsToUpdate);

                // Envía solo los registros actualizados a los clientes conectados
                const message = JSON.stringify({
                    message: 'Registro actualizado',
                    data: recordsToUpdate
                });

                wss.clients.forEach(client => {
                    if (client.readyState === WebSocket.OPEN) {
                        client.send(message); // Enviar solo los datos actualizados
                        console.log('Enviando registros actualizados al cliente:', message); // Mensaje que se envía
                    } else {
                        console.log('Cliente no está listo para recibir mensajes');
                    }
                });

                previousRecords = currentRecords; // Actualiza los registros anteriores
            }
        } catch (err) {
            console.error('Error al verificar los cambios:', err);
        }
    }, 5000); // Verificar cada 5 segundos
}

// Exportar las funciones necesarias
module.exports = {
    connectToDatabase,
    getAllRegistros,
    checkForChanges
};
