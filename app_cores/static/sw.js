self.addEventListener('fetch', (event) => {
  // Necessário para o navegador reconhecer como PWA
  event.respondWith(fetch(event.request));
});