<?php
$servername = "localhost"; 
$username = "root";   
$password = "ProyectoTele"; 
$dbname = "databasetele"; 

// Crear conexión
$conn = new mysqli($servername, $username, $password, $dbname);

// Comprobar conexión
if ($conn->connect_error) {
    die("Conexión fallida: " . $conn->connect_error);
}

// Verificar si se recibió un POST
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Obtener variables del POST
    $mensaje = isset($_POST['uuid']) ? $_POST['uuid'] : '';
    $ingreso = isset($_POST['ingreso']) ? $_POST['ingreso'] : '';

    // Mostrar los datos recibidos
    echo "Datos recibidos:<br>";
    echo "Método recibido: " . $_SERVER['REQUEST_METHOD'] . "<br>";
    echo "UUID: " . htmlspecialchars($mensaje) . "<br>";
    echo "Ingreso: " . htmlspecialchars($ingreso) . "<br>";

    // Verificar si el uuid ya existe
    $stmt = $conn->prepare("SELECT id FROM Registro WHERE uuid = ?");
    $stmt->bind_param("s", $mensaje);
    $stmt->execute();
    $stmt->store_result();

    if ($stmt->num_rows > 0) {
        // Si el uuid existe, actualiza el campo ingreso
        $stmt->close();
        $stmt = $conn->prepare("UPDATE Registro SET ingreso = ? WHERE uuid = ?");
        $stmt->bind_param("is", $ingreso, $mensaje); // 'is' indica un entero y una cadena
        if ($stmt->execute()) {
            echo "Ingreso actualizado correctamente.";
        } else {
            echo "Error al actualizar el ingreso: " . $stmt->error;
        }
    } else {
        // Si no existe, inserta un nuevo registro
        $stmt->close();
        $stmt = $conn->prepare("INSERT INTO Registro (uuid, ingreso) VALUES (?, ?)");
        $stmt->bind_param("si", $mensaje, $ingreso); // 'si' indica una cadena y un entero
        if ($stmt->execute()) {
            echo "Datos guardados correctamente.";
        } else {
            echo "Error al guardar los datos: " . $stmt->error;
        }
    }

    // Cerrar la declaración
    $stmt->close();
} else {
    echo "Método no permitido.";
}

// Cerrar la conexión
$conn->close();
?>





<!-- CREATE TABLE Registro ( 
    id INT(11) AUTO_INCREMENT PRIMARY KEY,
    uuid VARCHAR(255) NOT NULL, 
    ingreso  NOT NULL,
    fecha TIMESTAMP DEFAULT CURRENT_TIMESTAMP
); -->