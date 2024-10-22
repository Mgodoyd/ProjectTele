import { Component, OnInit, OnDestroy } from '@angular/core';
import { RouterOutlet } from '@angular/router';
import { WebSocketServiceService } from './web-socket-service.service';
import { CommonModule } from '@angular/common';
import { v4 as uuidv4 } from 'uuid';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [RouterOutlet, CommonModule],
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss']
})
export class AppComponent implements OnInit, OnDestroy {
  data: { uuid: string; uuidSesion: string; ingreso: boolean; tiempo: number; cerrado?: boolean }[] = [];
  activeSessions = new Map<string, any>(); // Para manejar sesiones activas

  constructor(private webSocketService: WebSocketServiceService) { }

  ngOnInit() {
    console.log('Component initialized');
    this.webSocketService.connect();
    this.webSocketService.onMessage().subscribe((message: any) => {
      console.log('WebSocket message received:', message);
      this.handleWebSocketMessage(message);
    });
  }

  handleWebSocketMessage(message: any) {
    if (message && message.data && message.data.length > 0) {
      message.data.forEach((item: any) => {
        console.log('Handling WebSocket message item:', item);

        if (item.ingreso) {
          // Usar el UUID que viene del servidor
          const serverUuid = item.uuid;
          const uuidSesion = uuidv4();  // Generar un UUID para la sesión si es necesario

          // Agregar el UUID del servidor y el UUID de la sesión
          this.data.push({
            uuid: serverUuid,         // Usar el UUID recibido
            uuidSesion: uuidSesion,   // Generar o usar un UUID para la sesión
            ingreso: true,
            tiempo: 0
          });

          this.startTimer(serverUuid);  // Iniciar el temporizador con el UUID del servidor
          console.log(`New session started with Server UUID: ${serverUuid}, Session UUID: ${uuidSesion}`);
        } else {
          // Manejar la salida
          const session = this.data.find(s => s.ingreso === true && !s.cerrado);
          if (session) {
            this.endSession(session.uuid);
          }
        }
      });
    }
  }


  startTimer(uuid: string) {
    let accumulatedTime = 0;
    const interval = setInterval(() => {
      accumulatedTime += 1; // Incrementa el tiempo acumulado
      this.data.find(s => s.uuid === uuid)!.tiempo = accumulatedTime; // Actualiza el tiempo en la sesión
      console.log(`Updated elapsed time for UUID ${uuid}: ${accumulatedTime}`);
    }, 1000);

    this.activeSessions.set(uuid, interval);
  }

  endSession(uuid: string) {
    const interval = this.activeSessions.get(uuid);
    if (interval) {
      clearInterval(interval); // Detener el cronómetro
      this.activeSessions.delete(uuid); // Eliminar el intervalo
      const session = this.data.find(s => s.uuid === uuid);
      if (session) {
        session.ingreso = false; // Marcar como salida
        console.log(`Session ended for UUID ${uuid}. Time: ${session.tiempo} seconds`);
      }
    }
  }

  ngOnDestroy() {
    console.log('Component destroyed');
    this.webSocketService.disconnect();
    // Limpiar todos los intervalos al destruir el componente
    this.activeSessions.forEach((interval, uuid) => {
      clearInterval(interval);
      console.log(`Cleared interval for UUID: ${uuid}`);
    });
  }
}
