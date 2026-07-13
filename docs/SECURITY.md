# Seguridad de EcoCycle

Auditoría de los backends (.NET y Visor) realizada el 2026-07-12, con las
correcciones aplicadas y las acciones manuales pendientes.

## Vulnerabilidades corregidas

### Backend .NET (`EcoCycle/backend`)

| # | Vulnerabilidad | Corrección |
|---|---|---|
| 1 | `POST /api/Auth/login` devolvía el perfil completo **con solo conocer el email** (sin verificar contraseña) | Ahora verifica email+contraseña contra Firebase Authentication (`FirebaseAuthService`) y responde "Credenciales inválidas" genérico |
| 2 | `POST /api/Auth/registro` aceptaba `rol` del cliente → **cualquier usuario podía registrarse como admin** | El rol se fuerza a `cliente` salvo lista blanca |
| 3 | `POST /api/SesionReciclaje` era anónimo → **cualquiera podía acreditarse puntos ilimitados** | Requiere `X-Api-Key` (máquina) o JWT (terminal web) |
| 4 | `GET/PUT/DELETE /api/Usuario/{id}` y `/todos` solo pedían estar logueado → **cualquier cliente podía listar usuarios, cambiar roles (escalada a admin) o borrar cuentas** | Filtro `[AdminOnly]` que valida el rol en Firestore |
| 5 | `Proveedor`, `CompraProveedor`, `Reportes`, `Comentarios` **sin autenticación** | `[Authorize]` (+ `[AdminOnly]` en proveedores/compras) |
| 6 | Contraseña SMTP de Gmail **versionada en `appsettings.json`** | Se lee de `SMTP_PASSWORD` / `Email__SmtpPassword` (entorno); el archivo ya no la contiene |
| 7 | La contraseña del usuario se **enviaba en texto plano por correo** de bienvenida | El correo ya no incluye la contraseña |
| 8 | Contraseñas temporales generadas con `Random` (predecible) y **logueadas en consola** | `RandomNumberGenerator` y sin logs de secretos |
| 9 | Los errores 500 devolvían `ex.Message` (detalles internos) | Mensajes genéricos + log estructurado con `ILogger` |
| 10 | CORS fijo a `localhost:4200` | Orígenes configurables (`AllowedOrigins`) |
| 11 | Sin límite de intentos de login | Rate limiting .NET (10/min por IP) + zona `limit_req` en nginx |

### Visor (`Visor/`, NestJS + YOLO)

| # | Vulnerabilidad | Corrección |
|---|---|---|
| 1 | **Todos los endpoints abiertos** (abrir compuerta, sumar botellas, limpiar sesión de cualquier máquina) | `MachineKeyGuard` global: exige `X-Api-Key` = `MACHINE_API_KEY`; tablet y ESP32 la envían (via `generate_configs.py`) |
| 2 | Clave privada de Firebase (`firebase-service-account.json`) **commiteada en el repo** | Quitada del control de versiones + `.gitignore` (⚠️ ver pendientes: rotarla) |
| 3 | Llamadas a Realtime Database **sin autenticación** (requiere reglas públicas) | El Visor ahora obtiene un token OAuth2 con la cuenta de servicio; ya se pueden cerrar las reglas |
| 4 | Subida de imágenes sin límite de tamaño (DoS de memoria/disco) | Límite 10 MB en NestJS y 20 MB en YOLO |
| 5 | Toda imagen recibida se guardaba en disco sin límite | Solo si `DEBUG_IMAGES=true` |
| 6 | Puerto YOLO (8000) publicado a internet en el VPS | Ligado a `127.0.0.1` (dev) o sin publicar (prod) |
| 7 | `.venv/` versionado en git | Quitado + `.gitignore` |

### Configuración / infraestructura

| # | Problema | Corrección |
|---|---|---|
| 1 | `ecocycle.env` (WiFi, API keys) versionado | Quitado del repo; plantilla `ecocycle.env.example` |
| 2 | Backend .NET en producción corriendo con `dotnet watch` sobre imagen SDK | `Dockerfile` multi-stage con imagen runtime (`docker-compose.prod.yml`) |
| 3 | Front en producción con `ng serve` (dev server) | Build de producción servido por nginx |
| 4 | Todos los puertos internos publicados en la IP pública | En prod solo `127.0.0.1`; nginx es el único punto de entrada |
| 5 | nginx sin límites ni cabeceras | `client_max_body_size`, rate limit de login, HSTS, X-Frame-Options, etc. |

## ⚠️ Acciones manuales pendientes (importante)

Estas NO se pueden hacer desde el código; hay que hacerlas en las consolas:

1. **Rotar la clave de servicio de Firebase.** La clave
   `firebase-service-account.json` estuvo commiteada en el historial de git del
   repo Visor. Aunque ya no está versionada, sigue en el historial: en Google
   Cloud Console → IAM → Cuentas de servicio → eliminar esa clave y generar una
   nueva; reemplazar los dos archivos locales.
   **Nota:** al probar la autenticación OAuth del Visor, Google respondió
   `Invalid JWT Signature` con la clave local actual (id `edba9ae7…`): parece
   que esa clave ya fue revocada. Hasta que pongas una clave nueva, el Visor
   sigue accediendo a Realtime Database sin autenticar (reglas públicas), igual
   que antes. Con la clave nueva, la autenticación se activa sola.
2. **Rotar la contraseña de aplicación de Gmail** (`eqabykhgxbglhzic` estuvo en
   el historial de `appsettings.json`). Google Account → Seguridad →
   Contraseñas de aplicación: revocar y crear una nueva; ponerla en
   `ecocycle.env` (`SMTP_PASSWORD`).
3. **Cerrar las reglas de Realtime Database.** El Visor ya se autentica, así que
   en Firebase Console → Realtime Database → Reglas:
   ```json
   { "rules": { ".read": "auth != null", ".write": "auth != null" } }
   ```
   Ojo: la app móvil y la tablet también acceden a RTDB; verificar que usan
   Firebase Auth antes de cerrar del todo (probar en local primero).
4. **Revisar reglas de Firestore** con el mismo criterio.
5. (Opcional) Limpiar el historial de git de los secretos con
   `git filter-repo` si el repo va a hacerse público.

## Modelo de amenaza rápido

- **Web/API pública**: autenticación Firebase JWT + roles en Firestore
  (`[AdminOnly]`). Login con verificación real de credenciales y rate limit.
- **Dispositivos IoT**: API key compartida (`X-Api-Key`) sobre HTTPS. Si se
  filtra, se rota en `ecocycle.env` y se regeneran configs.
- **Secretos**: viven en `ecocycle.env` y archivos de clave no versionados.
- **Red**: en el VPS solo nginx (80/443) escucha al exterior.
