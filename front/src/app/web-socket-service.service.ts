import { Injectable } from '@angular/core';
import { Subject } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class WebSocketServiceService {
  private socket!: WebSocket; // Declara una variable WebSocket
  private messageSubject = new Subject<any>(); // Crea un Subject para emitir mensajes

  constructor() { }

  // Método para establecer la conexión
  connect() {
    this.socket = new WebSocket('ws://localhost:3000'); // Conecta al servidor WebSocket

    this.socket.onopen = () => {
      console.log('Conexión WebSocket establecida'); // Mensaje al abrir la conexión
    };

    this.socket.onmessage = (event) => {
      // Procesa el mensaje recibido y lo envía al Subject
      const response = JSON.parse(event.data); // Asumiendo que los datos son JSON
      this.messageSubject.next(response); // Emite el mensaje recibido
      console.log('Mensaje recibido:', response); // Log del mensaje recibido
    };

    this.socket.onerror = (error) => {
      console.error('Error en WebSocket:', error); // Maneja errores de WebSocket
    };

    this.socket.onclose = () => {
      console.log('Conexión WebSocket cerrada'); // Mensaje al cerrar la conexión
    };
  }

  // Método para que los componentes se suscriban a los mensajes
  onMessage() {
    return this.messageSubject.asObservable(); // Devuelve un observable para que otros puedan suscribirse
  }

  // Método para cerrar la conexión
  disconnect() {
    if (this.socket) {
      this.socket.close(); // Cierra la conexión si existe
    }
  }
}
