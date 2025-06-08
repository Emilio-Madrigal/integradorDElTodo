const estados = {
  foco: false,
  bomba: false,
  fan1: false,
  fan2: false
};

function toggle(elemento) {
  const checkbox = document.getElementById(`switch-${elemento}`);
  const estado = checkbox.checked;
  estados[elemento] = estado;

  actualizarImagen(elemento, estado);

  // Opcional: enviar estado al servidor o ESP32
  // fetch(`/api/${elemento}?estado=${estado ? 'on' : 'off'}`);
}

function actualizarImagen(elemento, estado) {
  const img = document.getElementById(`img-${elemento}`);
  const estadoStr = estado ? 'on' : 'off';

  let path = `img/${elemento}_${estadoStr}`;

  
  if (elemento.includes("fan") && estado) {
    path = `img/fan_on.gif`;
  }

  img.src = `${path}.png`.replace('.gif.png', '.gif'); // Corrige si es .gif
}

function actualizarSensores() {
  // Simulación
  document.getElementById('temp').innerText = `${(20 + Math.random() * 10).toFixed(1)} °C`;
  document.getElementById('humedad').innerText = `${(50 + Math.random() * 20).toFixed(1)} %`;

  setTimeout(actualizarSensores, 5000);
}

actualizarSensores();
