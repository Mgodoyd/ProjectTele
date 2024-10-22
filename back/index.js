const express = require('express');
const cors = require('cors'); // Importa el middleware cors
const WebSocket = require('ws'); // Importa WebSocket
const { connectToDatabase, checkForChanges } = require('./db'); // Importar funciones de db.js

const app = express();
const port = 3000;

// Configura el middleware cors con opciones específicas
const corsOptions = {
    origin: 'http://localhost:4200', // Reemplaza con el origen de tu aplicación cliente
    methods: '*',
    credentials: true,
    optionsSuccessStatus: 204
};

app.use(cors(corsOptions));
app.use(express.json());

// Inicia el servidor HTTP
const server = app.listen(port, () => {
    console.log(`Servidor HTTP escuchando en http://localhost:${port}`);
});

// Configura el servidor WebSocket
const wss = new WebSocket.Server({ server });

// Cuando un cliente se conecta al WebSocket
wss.on('connection', (ws) => {
    console.log('Cliente WebSocket conectado');

    ws.on('close', () => {
        console.log('Cliente WebSocket desconectado');
    });
});

// Conectar a la base de datos
connectToDatabase();

// Llama a la función para verificar cambios en la base de datos y enviar mensajes a los clientes
checkForChanges(wss);
