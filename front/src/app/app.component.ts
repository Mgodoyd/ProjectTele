import { Component, OnInit, OnDestroy } from '@angular/core';
import { RouterOutlet } from '@angular/router';
import { WebSocketServiceService } from './web-socket-service.service';
import { CommonModule } from '@angular/common';
import { v4 as uuidv4 } from 'uuid'; // Importar la función para generar UUIDs

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [RouterOutlet, CommonModule],
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss']
})
export class AppComponent implements OnInit, OnDestroy {
  data: any[] = [];
  previousStateMap = new Map<string, boolean>();
  intervalMap = new Map<string, any>();
  elapsedTimeMap = new Map<string, number>();
  lastSessionTimeMap = new Map<string, number[]>(); // Mapa para guardar los tiempos de las sesiones

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
        this.data.push(item);
        this.manageTimers(item);
      });
    }
  }

  manageTimers(item: any) {
    const previousState = this.previousStateMap.get(item.uuid);
    let accumulatedTime = this.elapsedTimeMap.get(item.uuid) || 0;
    if (item.ingreso) {
      // Si es un ingreso y no hay cronómetro corriendo o si es un nuevo ingreso después de una salida
      if (!this.intervalMap.has(item.uuid) || previousState === false) {
        this.elapsedTimeMap.set(item.uuid, accumulatedTime);
        const interval = setInterval(() => {
          accumulatedTime += 1; // Incrementa el tiempo acumulado
          this.elapsedTimeMap.set(item.uuid, accumulatedTime); // Actualiza el tiempo acumulado en el mapa
          console.log(`Updated elapsed time for UUID ${item.uuid}: ${accumulatedTime}`);
        }, 1000);
        this.intervalMap.set(item.uuid, interval); // Almacenar el intervalo
        console.log(`Timer started for UUID: ${item.uuid}`);
      }
    } else {
      // Si es una salida, detener el cronómetro
      const interval = this.intervalMap.get(item.uuid);
      if (interval) {
        clearInterval(interval); // Detener el cronómetro
        this.intervalMap.delete(item.uuid); // Eliminar el intervalo
        console.log(`Timer stopped for UUID: ${item.uuid}`);
        // Guardar el tiempo acumulado en el historial de las sesiones
        const history = this.lastSessionTimeMap.get(item.uuid) || [];
        history.push(accumulatedTime); // Añadir el tiempo acumulado actual al historial
        this.lastSessionTimeMap.set(item.uuid, history);
        console.log(`Final accumulated time for UUID ${item.uuid}: ${accumulatedTime}`);
        console.log(`Updated history for UUID ${item.uuid}: ${history}`);
        // Resetear el tiempo acumulado para el siguiente ciclo de ingreso/salida
        this.elapsedTimeMap.set(item.uuid, 0);
      }
    }
    // Actualizar el estado anterior para la próxima verificación
    this.previousStateMap.set(item.uuid, item.ingreso);
    console.log(`Updated previous state for UUID ${item.uuid}: ${item.ingreso}`);
  }

  ngOnDestroy() {
    console.log('Component destroyed');
    this.webSocketService.disconnect();
    // Limpiar todos los intervalos al destruir el componente
    this.intervalMap.forEach((interval, uuid) => {
      clearInterval(interval);
      console.log(`Cleared interval for UUID: ${uuid}`);
    });
  }
}
