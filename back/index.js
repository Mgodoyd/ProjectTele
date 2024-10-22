const express = require('express');
const cors = require('cors'); 
const WebSocket = require('ws'); 
const { connectToDatabase, checkForChanges } = require('./db'); 

const app = express();
const port = 3000;

// Configura el middleware cors con opciones específicas
const corsOptions = {
    origin: 'http://localhost:4200', 
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
