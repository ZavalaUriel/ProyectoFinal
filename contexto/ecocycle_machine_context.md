# Máquina Inteligente de Reciclaje — Contexto de Diseño General

## Descripción General

La máquina es un módulo autónomo de reciclaje inteligente de botellas PET. Combina validación por cámara con IA, mecanismo anti-robo de doble compuerta, depósito aislado, y una pantalla interactiva para guiar al usuario. El flujo completo ocurre de manera automática: el usuario introduce la botella, el sistema la valida, y la acepta o rechaza sin intervención manual adicional.

---

## Dimensiones y Estructura Física

| Atributo | Valor |
|---|---|
| Ancho | 900 mm |
| Fondo | 700 mm |
| Alto | 1800 mm |
| Estructura | Lámina de acero al carbono / Aluminio |
| Acabado | Pintura electrostática |
| Nivelación | Patas ajustables |
| Apertura de entrada | Ø 300 mm con compuerta basculante |
| Conectividad | WiFi / 4G / Ethernet |

### Vistas

- **Vista Frontal (900 mm ancho):** Pantalla interactiva en la parte superior, apertura circular de botellas en el centro, sección de depósito aislado en la parte inferior.
- **Vista Lateral Derecha (700 mm fondo):** Acceso lateral; estructura contenedora sin componentes externos visibles.
- **Vista Trasera / Puerta de Servicio (900 mm ancho):** Puerta trasera de acceso total para extracción de botellas y mantenimiento. Incluye rejilla de ventilación en la parte superior trasera.
- **Vista Superior:** Planta rectangular de 900 × 700 mm.
- **Vista Inferior:** Base con patas ajustables y compartimento inferior.

---

## Componentes Principales

### 1. Pantalla Interactiva

- **Hardware:** Tablet Samsung Galaxy Tab A11+ (11")
- **Función:** Interfaz de usuario principal. Muestra instrucciones, estado del proceso, mensajes de validación y confirmación.
- **Posición:** Parte superior frontal de la máquina.
- **Conectividad:** La tablet se comunica con el sistema central vía red local (WiFi o Ethernet).

---

### 2. Sistema de Entrada de Botellas

- **Apertura:** Compuerta circular de Ø 300 mm.
- **Tipo de compuerta:** Basculante — puede operar automáticamente o de forma manual.
- **Posición de apertura:** La compuerta abre hacia arriba para permitir la introducción de la botella.
- **Material:** Estructura rígida, integrada al chasis frontal.

---

### 3. Sistema de Validación Inteligente

Compuesto por tres elementos que trabajan en conjunto:

#### 3a. Cámara + Iluminación LED
- Cámara posicionada en el interior del túnel de entrada.
- Iluminación LED para garantizar condiciones de imagen consistentes independientemente del entorno.
- Captura imagen del objeto al momento de ser introducido.

#### 3b. IA en Dispositivo (On-Device)
- Modelo de IA que corre localmente en el sistema embebido.
- Identifica si el objeto es una botella PET válida.
- Rechaza otros objetos (vidrio, latas, basura, etc.).
- Resultado binario: **VÁLIDO** / **NO VÁLIDO**.

#### 3c. Sensores Auxiliares
| Sensor | Función |
|---|---|
| Sensor Infrarrojo | Detecta objeto entrante (trigger de captura de imagen) |
| Celda de Carga | Verifica peso mínimo del objeto (filtra objetos muy ligeros o vacíos) |
| Sensor de Proximidad | Detecta presencia y posición del objeto en el túnel |

---

### 4. Mecanismo Anti-Robo y Privacidad (Doble Compuerta)

Sistema de dos compuertas en serie que impiden el acceso al depósito interno desde el exterior:

| Paso | Evento |
|---|---|
| 1 | La botella entra por la compuerta frontal (exterior) |
| 2 | La compuerta frontal se cierra — aísla el interior |
| 3 | La compuerta interna se abre — la botella cae al depósito cilíndrico |
| 4 | La compuerta interna se cierra — ambas compuertas quedan aisladas y no son accesibles desde el exterior simultáneamente |

**Principio de seguridad:** Nunca están ambas compuertas abiertas al mismo tiempo. El depósito interno nunca tiene línea de visión directa desde el exterior.

---

### 5. Depósito Interno Aislado

- **Forma:** Cilíndrica.
- **Tipo:** Aislado y privado — no accesible desde el frente ni los laterales.
- **Capacidad:** Alta, variable según tamaño de botellas.
- **Acceso:** Exclusivamente por la puerta trasera de servicio.
- **Función:** Almacena las botellas aceptadas de forma segura e higiénica.

---

### 6. Puerta Trasera de Servicio

- **Posición:** Panel trasero completo.
- **Función:** Acceso total al interior — permite extracción de botellas, limpieza y mantenimiento de componentes internos.
- **Estado operativo:** Cerrada durante operación normal; abierta solo por personal autorizado.

---

## Flujo de Uso (Proceso Completo)

```
[Usuario introduce botella]
        │
        ▼
[Sensor de Proximidad / IR detecta objeto]
        │
        ▼
[Cámara captura imagen con iluminación LED]
        │
        ▼
[Modelo IA analiza: ¿Es botella PET válida?]
        │
    ┌───┴───┐
   SÍ      NO
    │       │
    ▼       ▼
[Celda de  [Compuerta frontal
 carga      se abre — objeto
 verifica   devuelto al usuario]
 peso]
    │
    ▼
[Compuerta interna se abre]
    │
    ▼
[Botella cae al depósito cilíndrico]
    │
    ▼
[Compuerta interna se cierra]
    │
    ▼
[Pantalla muestra confirmación / puntos / recompensa]
    │
    ▼
[Proceso completado — máquina lista para siguiente botella]
```

---

## Características Principales (Resumen)

| Característica | Descripción |
|---|---|
| **Depósito Aislado** | Sistema cilíndrico interno que garantiza privacidad y seguridad del contenido |
| **Validación Inteligente** | Cámara con IA verifica si el objeto es una botella reciclable |
| **Anti-Robo** | Compuerta doble y túnel interno impiden acceso al depósito |
| **Pantalla Interactiva** | Samsung Galaxy Tab A11+ (11") para interacción y guía |
| **Acceso de Servicio** | Puerta trasera para extracción de botellas y mantenimiento |

---

## Arquitectura de Componentes y Conexiones

```
┌─────────────────────────────────────────────────────┐
│                  MÁQUINA ECOCYCLE                    │
│                                                      │
│  [Samsung Tab A11+]  ◄──── WiFi/Ethernet ────►  [API Backend]
│       (UI/UX)                                        │
│          │                                           │
│          │ (trigger validación / mostrar resultado)  │
│          ▼                                           │
│  [Sistema de Control Central / MCU]                  │
│          │                                           │
│    ┌─────┼──────────────────────┐                    │
│    │     │                      │                    │
│    ▼     ▼                      ▼                    │
│ [Cámara] [Sensor IR]       [Celda de carga]          │
│ [LED]    [Sensor Proximidad]                         │
│    │                            │                    │
│    ▼                            ▼                    │
│ [Modelo IA On-Device] ◄─── [Datos de sensores]       │
│    │                                                 │
│    ▼ (resultado)                                     │
│ [Controlador de Compuertas]                          │
│    │                                                 │
│    ├── [Compuerta Frontal / Exterior] (Servo/Motor)  │
│    └── [Compuerta Interna / Depósito] (Servo/Motor)  │
│                   │                                  │
│                   ▼                                  │
│         [Depósito Cilíndrico Aislado]                │
│                   │                                  │
│                   ▼                                  │
│        [Puerta Trasera de Servicio]                  │
└─────────────────────────────────────────────────────┘
```

---

## Notas para Implementación (Contexto de Desarrollo)

- La **validación IA** puede implementarse on-device (Raspberry Pi / ESP32 con modelo cuantizado) o delegarse al backend vía API si hay conectividad garantizada.
- El **sensor IR** actúa como trigger principal: cuando detecta objeto, inicia la secuencia de captura. Evita ciclos de cámara activa innecesarios.
- La **celda de carga** es una segunda validación de seguridad post-IA: descarta objetos que aunque parezcan botellas, tienen peso fuera del rango esperado.
- El **mecanismo de doble compuerta** debe tener lógica de bloqueo mutuo (interlock): nunca se envía señal de apertura a compuerta interna si compuerta frontal no está confirmada como cerrada.
- La **pantalla (Tab A11+)** es el canal de feedback al usuario: debe mostrar el estado en tiempo real (esperando, validando, aceptado, rechazado, error).
- El **acceso de servicio trasero** debe tener sensor de puerta abierta para inhibir operación mientras esté abierto (seguridad operativa).
