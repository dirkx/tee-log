static const char page[] PROGMEM = R"(
<html>
  <head>
      <title>Log</title>
  </head> 
    <style> 
        #log { font-family: 'Courier New', monospace; white-space: pre; }
    </style>
  <script language="javascript">
     var gateway = 'ws://'+location.host+'%s';
     var websocket;
     window.addEventListener('load', onload);

     function onload(event) { initWebSocket(); }
     function onOpen(event) { websocket.send("getHistory"); }
     function onClose(event) { setTimeout(initWebSocket, 2000); }
     function onMessage(event) { addLoglines(event.data); }

     function initWebSocket() {
         websocket = new WebSocket(gateway);
         websocket.onopen = onOpen;
         websocket.onclose = onClose;
         websocket.onmessage = onMessage;
     }
     function enc(str) {
         const map = { '<': '&lt;', '>': '&gt;', '&', '&amp;' };
         return str.replace(/<>&/,function(match) { return map[match]; });
     }
     function addLogLines(str) {
	 isAtEnd = (window.innerHeight + window.pageYOffset) >= document.body.offsetHeight - 4; 
         document.getElementById('log').innerHTML += enc(str);
         if (isAtEnd) window.scrollTo(0,document.body.scrollHeight); 
     }
</script>
	<body>
		<div id=log></div>
	</body>
</html>
)";


