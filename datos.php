<?php
// datos.php

// Lee el cuerpo del POST en JSON
$input = file_get_contents("php://input");
$data = json_decode($input, true);

// Imprime en consola de VSCode
error_log("Datos recibidos del HTML:");
error_log(print_r($data, true));

// También puedes enviar una respuesta al navegador
echo "Datos recibidos correctamente.";
