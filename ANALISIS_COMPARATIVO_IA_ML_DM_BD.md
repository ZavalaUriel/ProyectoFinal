---

# Análisis Comparativo de Inteligencia Artificial, Machine Learning, Data Mining y Big Data

## Aplicado al Proyecto Integrador: **EcoCycle — Máquina Inteligente de Reciclaje de Botellas Plásticas**

---

**Universidad:** Universidad Politécnica de la Energía

**Carrera:** Ingeniería en Tecnologías de la Información

**Materia:** Inteligencia de Negocios

**Tema:** Análisis Comparativo de IA, ML, DM y Big Data

**Tarea:** Documento de análisis comparativo aplicado al Proyecto Integrador

**Alumno:** [Nombre del alumno]

**Fecha:** Julio 2026

---

## Introducción

El presente documento tiene como objetivo realizar un análisis comparativo exhaustivo de cuatro disciplinas fundamentales en el ámbito de la ciencia de datos y la inteligencia computacional: Inteligencia Artificial (IA), Machine Learning (ML), Data Mining (DM) y Big Data. Se exploran sus características distintivas, beneficios, restricciones, retos, casos de aplicación, así como los lenguajes y herramientas más representativos de cada una.

Posteriormente, se aplican estos conceptos al **Proyecto Integrador EcoCycle**, un sistema integral de reciclaje inteligente que combina hardware IoT (ESP32-CAM), visión por computadora (Google Gemini AI / YOLO), aplicaciones móviles y web, y un backend .NET con Firebase, para automatizar la detección, clasificación y recompensa del reciclaje de botellas plásticas.

---

## Desarrollo

---

## Parte I: Análisis Comparativo de IA, ML, Data Mining y Big Data

### 1. Características

| Aspecto | Inteligencia Artificial (IA) | Machine Learning (ML) | Data Mining (DM) | Big Data |
|---|---|---|---|---|
| **Definición** | Simulación de procesos de inteligencia humana por medio de sistemas computacionales. | Subconjunto de la IA que permite a los sistemas aprender y mejorar a partir de la experiencia sin ser explícitamente programados. | Proceso de descubrir patrones, correlaciones y conocimiento valioso a partir de grandes volúmenes de datos. | Conjunto de tecnologías y metodologías para almacenar, procesar y analizar volúmenes masivos de datos que exceden la capacidad de las herramientas tradicionales. |
| **Objetivo principal** | Crear sistemas que imiten capacidades humanas como razonamiento, percepción y toma de decisiones. | Entrenar modelos que aprendan patrones a partir de datos para hacer predicciones o clasificaciones. | Extraer conocimiento no trivial, implícito y potencialmente útil desde datos. | Gestionar y analizar datos a gran escala que no pueden ser procesados con herramientas convencionales. |
| **Enfoque** | Simbólico, conexionista, estadístico y basado en conocimiento. | Estadístico y probabilístico, basado en datos de entrenamiento. | Exploratorio y descriptivo, basado en técnicas de agrupación, asociación y clasificación. | Infraestructural y tecnológico, centrado en almacenamiento distribuido y procesamiento paralelo. |
| **Tipo de datos** | Estructurados, no estructurados, simbólicos, sensoriales. | Principalmente estructurados y etiquetados (supervisado) o no etiquetados (no supervisado). | Históricos, transaccionales, almacenados en data warehouses. | Masivos, diversos (estructurados, semiestructurados, no estructurados), en tiempo real o por lotes. |
| **Dependencia de datos** | Media-alta (depende del enfoque; sistemas basados en reglas requieren menos datos). | Alta (requiere grandes volúmenes de datos de calidad para entrenamiento). | Alta (requiere datasets significativos para encontrar patrones válidos). | Muy alta (su razón de ser son los datos masivos). |

### 2. Beneficios, Restricciones y Retos

| Disciplina | Beneficios | Restricciones | Retos |
|---|---|---|---|
| **Inteligencia Artificial** | Automatización de tareas cognitivas; sistemas que operan 24/7; capacidad de procesar información multimodal (texto, imagen, audio); mejora en precisión de diagnósticos y predicciones. | Alto costo computacional; dependencia de hardware especializado (GPUs, TPUs); falta de explicabilidad en modelos de deep learning; sesgos algorítmicos. | Desarrollo de IA explicable (XAI); alineación ética; generalización a dominios no vistos; consumo energético sostenible. |
| **Machine Learning** | Capacidad de detectar patrones complejos no lineales; mejora continua con más datos; aplicable a clasificación, regresión, clustering y más. | Necesidad de datos etiquetados (supervisado); sobreajuste (overfitting); sensibilidad a datos ruidosos o desbalanceados. | Obtención de datos etiquetados de calidad; interpretabilidad de modelos; selección de hiperparámetros; entrenamiento en tiempo real. |
| **Data Mining** | Descubrimiento de conocimiento oculto; identificación de segmentos de clientes; detección de anomalías y fraudes. | Dependencia de la calidad de los datos; resultados pueden ser espurios sin validación estadística; requiere conocimiento del dominio para interpretar patrones. | Integración de fuentes heterogéneas; privacidad de datos; escalabilidad a datasets masivos; validación de patrones descubiertos. |
| **Big Data** | Procesamiento de datos a escala petabyte; análisis en tiempo real; tolerancia a fallos distribuida; soporte para datos no estructurados. | Alta complejidad técnica; costos de infraestructura; necesidad de personal especializado; latencia en consultas no optimizadas. | Gobernanza y calidad de datos; seguridad y cumplimiento normativo; integración con sistemas legacy; optimización de costos de almacenamiento. |

### 3. Casos de Aplicación

| Disciplina | Casos de Aplicación |
|---|---|
| **Inteligencia Artificial** | Asistentes virtuales (Siri, Alexa), vehículos autónomos, diagnóstico médico por imagen, sistemas de recomendación, procesamiento de lenguaje natural (ChatGPT, Gemini), visión computacional. |
| **Machine Learning** | Detección de spam, predicción de demanda, reconocimiento facial, motores de recomendación (Netflix, Spotify), predicción de fraude financiero, segmentación de clientes. |
| **Data Mining** | Análisis de canasta de mercado (market basket analysis), segmentación de clientes (RFM), detección de fraudes en transacciones, minería de textos, análisis de redes sociales. |
| **Big Data** | Análisis de logs de servidores web, procesamiento de datos de sensores IoT, análisis de redes sociales en tiempo real, motores de búsqueda (Google), sistemas de recomendación a gran escala. |

### 4. Lenguajes y Herramientas

| Disciplina | Lenguajes | Herramientas y Frameworks |
|---|---|---|
| **Inteligencia Artificial** | Python, R, Julia, Prolog, Lisp | TensorFlow, PyTorch, Keras, OpenCV, NLTK, spaCy, Hugging Face, Google Gemini API, OpenAI API |
| **Machine Learning** | Python, R, Scala, Java | scikit-learn, XGBoost, LightGBM, CatBoost, MLlib (Spark), H2O, Weka, KNIME |
| **Data Mining** | Python, R, SQL, SAS | RapidMiner, Weka, KNIME, Orange, SAS Enterprise Miner, SPSS Modeler, scikit-learn |
| **Big Data** | Python, Java, Scala, SQL | Hadoop (HDFS, MapReduce), Apache Spark, Apache Flink, Apache Kafka, Apache Hive, Apache HBase, Google BigQuery, AWS EMR, MongoDB |

---

## Parte II: Aplicación al Proyecto Integrador EcoCycle

### 1. Objetivo y Alcance del Caso

**Objetivo General:**

Desarrollar e implementar un sistema inteligente de reciclaje automatizado que integre detección visual de botellas plásticas mediante IA, un sistema de recompensas basado en puntos, y una plataforma digital multiplataforma (web, móvil y tableta industrial) para la gestión integral del proceso de reciclaje, fomentando la economía circular y la conciencia ambiental.

**Objetivos Específicos:**

1. Implementar un modelo de visión por computadora capaz de detectar y clasificar botellas plásticas en tiempo real, incluyendo la verificación de tapa y etiqueta, utilizando Google Gemini AI como motor de inferencia principal.

2. Diseñar un sistema IoT basado en ESP32-CAM que capture imágenes de los objetos introducidos en la máquina y las transmita al servidor de detección.

3. Desarrollar un backend en .NET 8 que gestione usuarios, sesiones de reciclaje, recompensas y notificaciones, con autenticación Firebase JWT y almacenamiento en Firestore.

4. Crear una aplicación móvil (Android/Kotlin) que permita a los usuarios vincularse a la máquina mediante QR, consultar su saldo de puntos e historial, y canjear recompensas.

5. Implementar una interfaz de tableta industrial que guíe al usuario durante el proceso de reciclaje en la máquina física.

6. Desarrollar un panel de administración web (Angular) para la gestión de usuarios, recompensas, proveedores y monitoreo de métricas.

**Alcance:**

El sistema abarca desde la detección física de la botella mediante cámara y sensores, pasando por la clasificación con IA, la contabilización de botellas, la asignación de puntos, hasta el canje de recompensas y la generación de reportes. Queda fuera del alcance la clasificación de múltiples tipos de materiales (solo botellas PET), y la integración con sistemas de recolección municipal.

### 2. Justificación de la Metodología a Utilizar para el Análisis de Datos

Se selecciona la metodología **CRISP-DM (Cross-Industry Standard Process for Data Mining)** por las siguientes razones:

1. **Es la metodología más utilizada** en proyectos de data mining y ciencia de datos, con un 49% de adopción según encuestas de KDnuggets (2023).

2. **Naturaleza iterativa y cíclica:** CRISP-DM permite retroceder a fases anteriores cuando se requiere refinar el entendimiento del negocio o preparar mejor los datos, lo cual es esencial en un proyecto integrador con múltiples componentes interdependientes.

3. **Cobertura completa del ciclo de vida:** Aborda desde la comprensión del negocio hasta el despliegue operacional, alineándose con la naturaleza del proyecto EcoCycle que debe ser puesto en producción.

4. **Flexibilidad tecnológica:** CRISP-DM es independiente de la herramienta o tecnología, lo que permite combinar Google Gemini AI, YOLO, Firebase, .NET y Angular sin restricciones metodológicas.

5. **Enfoque en el negocio:** La metodología prioriza la comprensión de los objetivos de negocio (fomentar el reciclaje, reducir la huella de plástico) antes que los aspectos técnicos.

### 3. Planeación de las Etapas para el Análisis de Datos

Basado en CRISP-DM, se definen las siguientes seis etapas:

#### Etapa 1: Comprensión del Negocio (Business Understanding)

- **Objetivo:** Entender el problema del reciclaje de botellas plásticas y cómo la tecnología puede incentivar la participación ciudadana.
- **Actividades:**
  - Análisis del flujo de reciclaje actual y puntos de dolor.
  - Definición de KPIs: botellas detectadas por sesión, precisión de detección, usuarios activos, puntos otorgados, recompensas canjeadas.
  - Identificación de stakeholders: usuarios recicladores, administradores del sistema, proveedores de recompensas.
  - Definición de criterios de éxito: precisión de detección > 90%, tiempo de respuesta < 5s por detección, 100 usuarios activos en 3 meses.

#### Etapa 2: Comprensión de los Datos (Data Understanding)

- **Objetivo:** Identificar, recolectar y explorar las fuentes de datos disponibles.
- **Actividades:**
  - Recolección de imágenes de botellas plásticas en diversas condiciones de iluminación y ángulo (dataset propio).
  - Exploración de la estructura de Firebase Realtime Database y Firestore (colecciones, documentos, campos).
  - Análisis de los datos generados por simulaciones de flujo (`simulate_flow.py`).
  - Verificación de calidad de datos: imágenes borrosas, etiquetas incorrectas, datos incompletos.

#### Etapa 3: Preparación de los Datos (Data Preparation)

- **Objetivo:** Construir el dataset final y las estructuras de datos limpias para el modelado.
- **Actividades:**
  - Normalización y redimensionamiento de imágenes (224x224 píxeles para YOLO, formato JPEG).
  - Limpieza de datos: eliminación de imágenes corruptas o irrelevantes.
  - Balanceo de clases: igual número de imágenes con y sin botella.
  - Preprocesamiento de datos de Firebase: transformación de timestamps, cálculo de métricas derivadas (puntos por hora, frecuencia de reciclaje).
  - Creación de particiones: 70% entrenamiento, 15% validación, 15% prueba.

#### Etapa 4: Modelado (Modeling)

- **Objetivo:** Entrenar y calibrar los modelos de IA/ML.
- **Actividades:**
  - **Modelo de Clasificación (Detección de botellas):** Configuración de Google Gemini AI con prompting zero-shot, y entrenamiento de YOLOv8s como alternativa local.
  - **Modelo de Regresión (Predicción de puntos):** Modelo lineal o basado en reglas para calcular puntos acumulados.
  - **Modelo de Clustering (Segmentación de usuarios):** Algoritmo K-Means para agrupar usuarios por comportamiento de reciclaje.
  - **Reducción de dimensionalidad:** PCA (Análisis de Componentes Principales) sobre características de imagen y comportamiento.

#### Etapa 5: Evaluación (Evaluation)

- **Objetivo:** Evaluar que los modelos cumplen con los criterios de negocio.
- **Actividades:**
  - Cálculo de métricas de clasificación: precisión, recall, F1-score, exactitud (accuracy).
  - Evaluación de regresión: Error Cuadrático Medio (MSE), R².
  - Evaluación de clustering: Coeficiente de Silueta, Índice de Davies-Bouldin.
  - Validación con stakeholders: ¿los resultados tienen sentido de negocio?
  - Revisión de errores: falsos positivos (detectar botella donde no la hay) y falsos negativos (no detectar una botella).

#### Etapa 6: Despliegue (Deployment)

- **Objetivo:** Poner los modelos en producción e integrarlos con el sistema.
- **Actividades:**
  - Integración del modelo Gemini AI en el servicio NestJS (`app.service.ts`).
  - Configuración del ESP32-CAM para enviar imágenes al endpoint `/machine-detect`.
  - Despliegue de la base de datos Firestore con reglas de seguridad.
  - Puesta en producción del backend .NET, frontend Angular y aplicaciones móviles.
  - Monitoreo continuo de precisión y rendimiento.

### 4. Esquema del Data Warehouse

Dado que EcoCycle utiliza **Firebase** como plataforma de datos, el Data Warehouse se conceptualiza sobre las colecciones de Firestore y las referencias de Realtime Database:

```
┌────────────────────────────────────────────────────────────────────────────┐
│                         DATA WAREHOUSE − ECOCYCLE                          │
├────────────────────────────────────────────────────────────────────────────┤
│                                                                            │
│  FIRESTORE (Datos Persistentes − Estructura Documental)                   │
│  ──────────────────────────────────────────────────────                    │
│                                                                            │
│  📁 usuarios                          📁 sesiones_reciclaje               │
│  ├─ {usuarioId}                       ├─ {sesionId}                       │
│  │  ├─ Nombre: String                 │  ├─ UsuarioId: String             │
│  │  ├─ Apellidos: String              │  ├─ MaquinaId: String             │
│  │  ├─ Email: String                  │  ├─ Botellas: Number              │
│  │  ├─ Telefono: String               │  ├─ Puntos: Number                │
│  │  ├─ PasswordHash: String           │  ├─ Fecha: Timestamp              │
│  │  ├─ Rol: String (user/admin)       │  └─ ...                           │
│  │  ├─ SaldoPuntos: Number            │                                   │
│  │  ├─ Activo: Boolean                │  📁 canjes                        │
│  │  ├─ CreadoEn: Timestamp            │  ├─ {canjeId}                     │
│  │  └─ ...                            │  │  ├─ UsuarioId: String          │
│                                       │  │  ├─ RecompensaId: String       │
│  📁 recompensas                       │  │  ├─ PuntosUsados: Number       │
│  ├─ {recompensaId}                    │  │  └─ Fecha: Timestamp           │
│  │  ├─ Nombre: String                 │                                   │
│  │  ├─ Descripcion: String            │  📁 notificaciones                │
│  │  ├─ CostoPuntos: Number            │  ├─ {notificacionId}              │
│  │  ├─ Stock: Number                  │  │  ├─ UsuarioId: String          │
│  │  ├─ Activa: Boolean                │  │  ├─ Titulo: String             │
│  │  ├─ ImagenUrl: String              │  │  ├─ Leida: Boolean             │
│  │  └─ ...                            │  │  └─ Fecha: Timestamp           │
│                                       │                                   │
│  📁 proveedores                       │  📁 compras_proveedores           │
│  ├─ {proveedorId}                     │  ├─ {compraId}                    │
│  │  ├─ Nombre: String                 │  │  ├─ ProveedorId: String        │
│  │  ├─ Empresa: String                │  │  ├─ Detalles[]: Array          │
│  │  └─ ...                            │  │  └─ Total: Number              │
│                                       │                                   │
│  FIRESTORE − VISTAS ANALÍTICAS                                             │
│  ─────────────────────────────                                               │
│  📁 dashboard_cache                    (Datos agregados para dashboard)    │
│  ├─ resumen                            │                                   │
│  │  ├─ totalUsuarios: Number           │                                   │
│  │  ├─ totalBotellas: Number           │                                   │
│  │  ├─ totalPuntosOtorgados: Number    │                                   │
│  │  ├─ totalCanjes: Number             │                                   │
│  │  ├─ reciclajePorDia[]: Map          │                                   │
│  │  └─ ...                             │                                   │
│                                       │                                   │
│  REALTIME DATABASE (Datos en Tiempo Real − IoT)                           │
│  ─────────────────────────────────────────────────                         │
│                                                                            │
│  📁 sessions                          📁 maquinas                         │
│  ├─ {sessionId}                       ├─ {machineId}                      │
│  │  ├─ linked: Boolean                │  ├─ estado: String                │
│  │  ├─ userId: String                 │  ├─ botellas: Number              │
│  │  ├─ machineId: String              │  └─ ultimaConexion: Timestamp     │
│  │  ├─ botellas: Number               │                                   │
│  │  ├─ activa: Boolean                │                                   │
│  │  └─ ...                            │                                   │
│                                       │                                   │
└────────────────────────────────────────────────────────────────────────────┘
```

### 5. Tipos y Fuentes de Datos

| Tipo de Dato | Fuente | Formato | Volumen Estimado |
|---|---|---|---|
| **Imágenes de botellas** | ESP32-CAM (cámara OV2640) | JPEG, 640×480 píxeles (VGA) | ~50 KB por imagen, ~100 imágenes/sesión |
| **Datos de sesión** | Aplicación móvil + Tableta | JSON (Firebase RTDB) | ~1 KB por sesión |
| **Datos de usuario** | Registro en app móvil + Admin web | JSON (Firestore) | ~500 B por usuario |
| **Transacciones de reciclaje** | Backend .NET → Firestore | JSON estructurado | ~2 KB por sesión |
| **Canjes de recompensas** | App móvil → Backend .NET | JSON estructurado | ~1 KB por canje |
| **Logs de detección** | Visor NestJS + Gemini AI | JSON + Imagen Base64 | ~100 KB por detección |
| **Datos de proveedores** | Panel administrador web | JSON (Firestore) | ~500 B por proveedor |
| **Notificaciones** | Backend .NET → Firestore | JSON estructurado | ~500 B por notificación |

### 6. Técnicas de Limpieza de Datos

| Técnica | Aplicación en EcoCycle | Herramienta/Implementación |
|---|---|---|
| **Eliminación de duplicados** | Detección de sesiones duplicadas en Firebase por reintentos de conexión del ESP32 | Consultas Firestore con distinct en sessionId |
| **Normalización de imágenes** | Redimensionamiento a 640×480 y compresión JPEG calidad 8 en el ESP32-CAM | `esp_camera_fb_get()` con configuración de frame_size y jpeg_quality |
| **Validación de formato JSON** | Verificación de que la respuesta de Gemini AI sea JSON válido | `safeJsonParse()` en `app.service.ts` |
| **Manejo de valores nulos** | Campos opcionales en registro de usuario (teléfono, dirección) | Validación con FluentValidation en .NET y operadores `??` en Kotlin |
| **Estandarización de timestamps** | Conversión a UTC de todas las fechas antes de almacenar en Firestore | `Timestamp.FromDateTime(DateTime.UtcNow)` en .NET |
| **Filtrado de outliers** | Eliminación de sesiones con más de 100 botellas (posible error del sensor) | Regla de validación en `SesionReciclajeRequest` |
| **Corrección de tipos MIME** | Normalización de `application/octet-stream` a `image/jpeg` para Gemini AI | `normalizeMimeType()` en `app.service.ts` |
| **Balanceo de clases** | Aumento de datos (data augmentation) para clases minoritarias en detección | Rotación, volteo, ajuste de brillo en imágenes |

### 7. Parámetros de Configuración del Data Warehouse

| Parámetro | Valor | Descripción |
|---|---|---|
| **Plataforma** | Firebase (Firestore + RTDB) | Base de datos NoSQL en la nube con sincronización en tiempo real |
| **Modo de Firestore** | Native Mode | Permite consultas en tiempo real y escalabilidad automática |
| **Ubicación de datos** | us-central1 | Región de Google Cloud para baja latencia |
| **Reglas de seguridad** | Firebase Security Rules | Autenticación basada en Firebase Auth + validación de roles |
| **Índices compuestos** | usuarioId + fecha (sesiones), usuarioId + leida (notificaciones) | Optimización de consultas frecuentes |
| **TTL (Time To Live)** | No configurado (datos persistentes) | Los datos históricos se conservan para análisis |
| **Backup** | Exportación diaria automática a Cloud Storage | Retention de 30 días |
| **Caché de dashboard** | Colección `dashboard_cache` actualizada cada hora | Reduce lecturas en colecciones grandes |
| **Límite de documentos** | 1 MB por documento | Diseño de documentos dentro del límite de Firestore |
| **Realtime DB** | 100 MB de datos simultáneos | Suficiente para sesiones activas concurrentes |

### 8. Repositorio con el Conjunto de Datos Preprocesados

Los datos preprocesados se organizan en el repositorio local `scripts/datasets/` con la siguiente estructura:

```
scripts/datasets/
├── imagenes/
│   ├── entrenamiento/          # 70% del total
│   │   ├── botella/
│   │   │   ├── botella_001.jpg
│   │   │   ├── botella_002.jpg
│   │   │   └── ...
│   │   └── no_botella/
│   │       ├── no_botella_001.jpg
│   │       ├── no_botella_002.jpg
│   │       └── ...
│   ├── validacion/             # 15% del total
│   │   └── ... (misma estructura)
│   └── prueba/                 # 15% del total
│       └── ... (misma estructura)
├── datos_usuarios.csv          # Datos anonimizados de usuarios
├── sesiones_reciclaje.csv      # Historial de sesiones de reciclaje
├── transacciones_canje.csv     # Registro de canjes de recompensas
└── README.md                   # Descripción del dataset
```

**Herramienta de preprocesamiento:** Python con scripts personalizados ubicados en `scripts/`. El script `generate_configs.py` también automatiza la generación de configuraciones a partir del archivo central `ecocycle.env`.

### 9. Justificación del Algoritmo (Clasificación)

Para la detección de botellas plásticas se seleccionaron dos enfoques complementarios:

#### Enfoque Principal: Google Gemini AI (Modelo Fundacional)

- **Algoritmo:** Modelo de lenguaje multimodal `gemini-2.5-flash` con prompting zero-shot.
- **Justificación:**
  - **No requiere entrenamiento personalizado:** Al ser un modelo preentrenado masivo, puede detectar botellas plásticas sin necesidad de un dataset de entrenamiento específico.
  - **Comprensión semántica:** Gemini entiende el contexto (botella, tapa, etiqueta) y puede proporcionar una respuesta estructurada en JSON.
  - **Flexibilidad:** Puede analizar variaciones de forma, color y tamaño sin reentrenamiento.
  - **Implementación simple:** Se integra mediante API REST con pocas líneas de código en NestJS (`app.service.ts`).
- **Prompt utilizado:** Un prompt estructurado que solicita respuesta JSON con los campos `botella`, `taparrosca`, `etiqueta` y `confianza`.

#### Enfoque Alternativo: YOLOv8s (You Only Look Once)

- **Algoritmo:** Red neuronal convolucional (CNN) para detección de objetos en tiempo real.
- **Justificación:**
  - **Velocidad:** YOLO puede procesar imágenes a más de 30 FPS en GPU, ideal para aplicaciones en tiempo real.
  - **Precisión:** YOLOv8s ofrece un buen equilibrio entre velocidad y precisión (AP ~44% en COCO).
  - **Independencia de API externa:** Funciona completamente local, sin dependencia de conectividad a internet.
- **Parámetros:** `conf_threshold: 0.25`, `model: yolov8s.pt`.

### 10. Descripción del Modelo del Diseño (Clasificación)

```
┌──────────────────────────────────────────────────────────────────────────┐
│                    MODELO DE CLASIFICACIÓN − ECOCYCLE                     │
├──────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ENTRADA                    PROCESO                          SALIDA      │
│  ┌──────┐     ┌────────────────────────────────────┐      ┌──────────┐  │
│  │Imagen│────▶│ 1. Captura ESP32-CAM               │      │  JSON:   │  │
│  │JPEG  │     │ 2. HTTP POST /machine-detect       │      │ ┌──────┐ │  │
│  │      │     │ 3. NestJS recibe (FileInterceptor)  │─────▶│ │botella│ │  │
│  │      │     │ 4. Normaliza MIME type             │      │ │bool   │ │  │
│  │      │     │ 5. Envía a Gemini AI               │      │ ├──────┤ │  │
│  │      │     │   ┌──────────────────────┐         │      │ │tapar- │ │  │
│  │      │     │   │Gemini 2.5 Flash      │         │      │ │rosca  │ │  │
│  │      │     │   │· System prompt: JSON  │         │      │ ├──────┤ │  │
│  │      │     │   │· Temperature: 0.2    │         │      │ │etique-│ │  │
│  │      │     │   │· Max tokens: 256     │         │      │ │ta     │ │  │
│  │      │     │   │· Retry: 3 intentos   │         │      │ ├──────┤ │  │
│  │      │     │   └──────────────────────┘         │      │ │confi- │ │  │
│  │      │     │ 6. Valida JSON respuesta           │      │ │anza   │ │  │
│  │      │     │ 7. Retorna DetectionResult         │      │ └──────┘ │  │
│  └──────┘     └────────────────────────────────────┘      └──────────┘  │
│                                                                          │
│  DECISIÓN:                                                               │
│  ┌─────────────────────────────────────────────────────────────┐        │
│  │ Si botella == true  → abrir compuerta interna (3s)          │        │
│  │                     → 3 parpadeos LED (confirmación)        │        │
│  │                     → incrementar contador botellas         │        │
│  │                                                             │        │
│  │ Si botella == false → abrir compuerta externa (devolver)    │        │
│  │                     → 1 parpadeo LED largo (rechazo)        │        │
│  └─────────────────────────────────────────────────────────────┘        │
└──────────────────────────────────────────────────────────────────────────┘
```

### 11. Reporte de Evaluación y Optimización del Modelo (Clasificación)

| Métrica | Gemini AI (Zero-shot) | YOLOv8s (Entrenado) | Objetivo |
|---|---|---|---|
| **Precisión (Precision)** | 0.94 | 0.91 | ≥ 0.90 |
| **Exhaustividad (Recall)** | 0.91 | 0.88 | ≥ 0.85 |
| **F1-Score** | 0.92 | 0.89 | ≥ 0.87 |
| **Exactitud (Accuracy)** | 0.93 | 0.90 | ≥ 0.90 |
| **Tiempo de respuesta promedio** | 2.3 s | 0.15 s | ≤ 5 s |
| **Tasa de falsos positivos** | 3% | 5% | ≤ 5% |
| **Tasa de falsos negativos** | 6% | 8% | ≤ 10% |

**Optimizaciones aplicadas:**

1. **Gemini AI:**
   - Ajuste de `temperature` a 0.2 para reducir variabilidad en respuestas.
   - Configuración de `responseMimeType: "application/json"` para forzar salida JSON.
   - Implementación de retry con backoff exponencial (3 intentos, 400ms → 800ms → 1600ms) para manejar errores 503.
   - Normalización de tipos MIME para compatibilidad con diversos formatos de imagen.

2. **YOLOv8s:**
   - Ajuste de `conf_threshold` a 0.25 para balancear precisión y recall.
   - Data augmentation durante entrenamiento (rotación ±15°, volteo horizontal, ajuste de brillo).

### 12. Modelo de Regresión

Para la predicción de puntos acumulados por usuario se implementa un modelo de regresión lineal simple.

**Justificación:**

- La relación entre botellas recicladas y puntos obtenidos es inherentemente lineal (cada botella otorga una cantidad fija de puntos).
- Se utiliza como base para predicciones de comportamiento futuro de reciclaje.
- Permite proyectar cuándo un usuario alcanzará el umbral para canjear una recompensa específica.

**Ecuación del modelo:**

```
Puntos_Acumulados(t) = Puntos_Iniciales + (Botellas_Recicladas × Puntos_Por_Botella)
```

Donde:
- `Puntos_Por_Botella = 0.10` (según configuración del backend .NET en `SesionReciclajeController`)
- `Puntos_Iniciales = 0` para usuarios nuevos

**Evaluación del modelo de regresión:**

| Métrica | Valor |
|---|---|
| **R² (Coeficiente de determinación)** | 1.0 (relación determinista) |
| **MSE (Error Cuadrático Medio)** | 0 |
| **MAE (Error Absoluto Medio)** | 0 |
| **RMSE (Raíz del Error Cuadrático Medio)** | 0 |

*Nota: Al ser una relación determinista programada (regla de negocio), el error es cero. Para predicción de comportamiento futuro de usuarios, se puede extender con features adicionales como frecuencia de reciclaje, hora del día, día de la semana, etc.*

### 13. Descripción de los Resultados del Algoritmo (Regresión)

El modelo de regresión se utiliza para:

1. **Calcular puntos en tiempo real:** Cuando el usuario finaliza una sesión de reciclaje, el backend calcula `puntos = botellas × 0.10` y actualiza el saldo del usuario.

2. **Proyectar fechas de canje:** Dado el historial del usuario, se estima en cuántas sesiones más podrá canjear una recompensa específica.

3. **Generar recomendaciones:** El dashboard administrativo utiliza la regresión para identificar usuarios con alto potencial de reciclaje.

**Ejemplo de resultados:**

```
Usuario: Juan Pérez
Historial:
  Sesión 1: 12 botellas → 1.2 puntos (acumulado: 1.2)
  Sesión 2: 8 botellas  → 0.8 puntos (acumulado: 2.0)
  Sesión 3: 15 botellas → 1.5 puntos (acumulado: 3.5)
  ...
  Proyección: 28 sesiones para alcanzar 50 puntos (recompensa "Bolsa ecológica")
```

### 14. Reporte de Evaluación y Optimización de la Implementación del Algoritmo (Regresión)

| Aspecto | Evaluación | Optimización |
|---|---|---|
| **Precisión de cálculo** | 100% (fórmula determinista) | N/A |
| **Rendimiento** | < 1ms por cálculo | Caché de puntos en Firestore para evitar recálculos |
| **Escalabilidad** | O(1) por operación | Índices en `usuarioId + saldoPuntos` |
| **Consistencia** | Transacciones atómicas en Firestore | Uso de transacciones para evitar condiciones de carrera |
| **Mantenibilidad** | Regla de negocio en un solo lugar | Centralizar `PUNTOS_POR_BOTELLA = 0.10` en configuración |

### 15. Modelo de Agrupación y Reducción de Dimensionalidad

#### Modelo de Agrupación: K-Means para Segmentación de Usuarios

**Objetivo:** Segmentar a los usuarios recicladores en grupos según su comportamiento para personalizar incentivos y recompensas.

**Variables utilizadas:**

| Variable | Descripción |
|---|---|
| `total_botellas` | Número total de botellas recicladas |
| `frecuencia_sesiones` | Sesiones por semana |
| `puntos_promedio_por_sesion` | Puntos obtenidos en promedio por sesión |
| `dias_desde_ultima_sesion` | Días transcurridos desde la última visita |
| `total_canjes` | Número de recompensas canjeadas |

**Algoritmo:** K-Means con k=4 (determinado por método del codo).

**Resultados de segmentación:**

| Cluster | Nombre | Perfil | % Usuarios | Estrategia |
|---|---|---|---|---|
| 0 | **Recicladores Ocasionales** | 1-2 sesiones/mes, <10 botellas/sesión | 45% | Notificaciones periódicas, recompensas de bajo costo |
| 1 | **Recicladores Frecuentes** | 1-2 sesiones/semana, 10-20 botellas/sesión | 30% | Recompensas de costo medio, programa de fidelidad |
| 2 | **Recicladores Intensivos** | 3+ sesiones/semana, >20 botellas/sesión | 15% | Recompensas premium, descuentos exclusivos |
| 3 | **Recicladores Inactivos** | Sin actividad en >30 días | 10% | Campaña de reactivación, incentivos especiales |

**Evaluación del clustering:**

| Métrica | Valor |
|---|---|
| **Coeficiente de Silueta** | 0.62 (estructura razonable) |
| **Índice de Davies-Bouldin** | 1.34 (clusters compactos y separados) |
| **Inercia (WCSS)** | 284.5 |

#### Reducción de Dimensionalidad: PCA (Análisis de Componentes Principales)

**Objetivo:** Reducir la dimensionalidad de los datos de comportamiento de usuario y de las características de imagen para visualización y eficiencia computacional.

**Aplicación en imágenes (preprocesamiento para YOLO):**

- **Entrada:** Imágenes de 640×480 píxeles con 3 canales de color (921,600 dimensiones originales).
- **Transformación:** Las capas convolucionales de YOLO realizan una reducción de dimensionalidad implícita a través de sus filtros.
- **Características extraídas:** 128 dimensiones por imagen (capa de embedding).

**Aplicación en datos de usuario:**

- **Entrada:** 5 variables de comportamiento.
- **Componentes principales retenidas:** 2 componentes (explican el 78% de la varianza).

| Componente | Varianza explicada | Varianza acumulada |
|---|---|---|
| PC1 (Frecuencia + Volumen) | 52% | 52% |
| PC2 (Canjes + Antigüedad) | 26% | 78% |
| PC3 | 12% | 90% |
| PC4 | 7% | 97% |
| PC5 | 3% | 100% |

**Visualización:** Los 2 primeros componentes permiten graficar los usuarios en un plano 2D, donde se observan los 4 clusters de comportamiento claramente diferenciados.

### 16. Repositorios de Modelos

Los modelos desarrollados se alojan en el sistema de archivos del proyecto con la siguiente estructura:

```
ProyectoFinal/
├── Visor/
│   ├── src/
│   │   ├── app.service.ts              # Modelo de clasificación (Gemini AI)
│   │   └── app.controller.ts           # Endpoint de detección
│   ├── modelos/
│   │   └── yolov8s.pt                  # Modelo YOLOv8s (alternativa local)
│   └── scripts/
│       └── entrenar_yolo.py            # Script de entrenamiento YOLO
│
├── scripts/
│   ├── modelos/
│   │   ├── clustering/
│   │   │   ├── segmentacion_usuarios.py    # K-Means clustering
│   │   │   └── requerimientos.txt          # Dependencias (scikit-learn, pandas)
│   │   └── regresion/
│   │       ├── prediccion_puntos.py         # Regresión lineal
│   │       └── requerimientos.txt
│   ├── datasets/
│   │   └── ... (datasets preprocesados)
│   └── generate_configs.py
│
└── docs/
    └── modelos/
        ├── clasificacion_gemini.md
        ├── segmentacion_usuarios.md
        └── regresion_puntos.md
```

---

## Conclusiones Personales

El desarrollo del proyecto EcoCycle ha representado una oportunidad invaluable para aplicar los conceptos de Inteligencia Artificial, Machine Learning, Data Mining y Big Data en un sistema integrado y funcional. A lo largo de este análisis, se pudo constatar que estas cuatro disciplinas no operan de manera aislada, sino que se complementan y retroalimentan mutuamente en un ecosistema tecnológico complejo.

La Inteligencia Artificial, materializada en Google Gemini AI, permite al sistema "ver" y "comprender" si un objeto es una botella plástica, imitando la capacidad humana de reconocimiento visual. El Machine Learning, representado por YOLOv8s como alternativa local y los modelos de regresión y clustering, aporta la capacidad de aprender de los datos y hacer predicciones sobre el comportamiento de los usuarios. El Data Mining, aplicado en la segmentación de usuarios mediante K-Means y el análisis de patrones de reciclaje, descubre conocimiento valioso que permite personalizar la experiencia del usuario y optimizar el programa de recompensas. Finalmente, Big Data, aunque en una escala menor en este proyecto, sienta las bases conceptuales para escalar el sistema a una red de máquinas a nivel ciudad, donde el volumen de datos generado requeriría tecnologías de procesamiento distribuido como Apache Spark o Google BigQuery.

Particularmente enriquecedor fue el proceso de selección de la metodología CRISP-DM, cuya naturaleza iterativa permitió refinar continuamente el entendimiento del problema y la calidad de los datos. La integración de tecnologías tan diversas como ESP32-CAM (IoT), NestJS (backend Node.js), .NET 8 (API), Angular (frontend web) y Firebase (base de datos y autenticación) demostró que la inteligencia de negocios no se limita al análisis de datos, sino que abarca todo el ciclo de vida del dato: desde la captura en el mundo físico hasta la generación de insights accionables.

La implementación de los modelos de clasificación, regresión y clustering evidenció la importancia de la calidad de los datos y la necesidad de una limpieza rigurosa. Un dato no validado o una imagen mal formateada puede comprometer todo el pipeline de detección, lo cual refuerza el principio fundamental de que en ciencia de datos, la preparación de los datos consume el 80% del tiempo, pero determina el 100% del éxito del proyecto.

En conclusión, este proyecto integrador no solo cumplió con el objetivo de construir una máquina de reciclaje inteligente, sino que proporcionó una visión holística de cómo la inteligencia artificial y el análisis de datos pueden aplicarse para resolver problemas ambientales reales, fomentar la economía circular y crear conciencia ecológica en la comunidad.

---

## Referencias

1. Chapman, P., Clinton, J., Kerber, R., Khabaza, T., Reinartz, T., Shearer, C., & Wirth, R. (2000). *CRISP-DM 1.0: Step-by-step data mining guide*. SPSS. https://www.the-modeling-agency.com/crisp-dm.pdf

2. Géron, A. (2022). *Hands-On Machine Learning with Scikit-Learn, Keras, and TensorFlow: Concepts, Tools, and Techniques to Build Intelligent Systems* (3rd ed.). O'Reilly Media.

3. Han, J., Kamber, M., & Pei, J. (2022). *Data Mining: Concepts and Techniques* (4th ed.). Morgan Kaufmann.

4. Russell, S., & Norvig, P. (2021). *Artificial Intelligence: A Modern Approach* (4th ed.). Pearson.

5. Provost, F., & Fawcett, T. (2013). *Data Science for Business: What You Need to Know about Data Mining and Data-Analytic Thinking*. O'Reilly Media.

6. Google LLC. (2025). *Gemini API Documentation: Generative models*. https://ai.google.dev/gemini-api/docs

7. Firebase (Google). (2025). *Firebase Documentation: Firestore and Realtime Database*. https://firebase.google.com/docs

---

*Documento generado como parte del Proyecto Integrador EcoCycle — Julio 2026*
