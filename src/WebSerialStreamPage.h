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
     function onMessage(event) {
	 var isAtEnd = (window.innerHeight + window.pageYOffset) >= document.body.offsetHeight - 4; 
         tag = document.getElementById('log')
         while (tag.innerHTML.length > 1014*1024*5) {
 		tag.innerHTML.slice(0, tag.innerHTML.indexOf("\n"));
	 };
 	 tag.innerHTML += htmlenc(event.data);
         if (isAtEnd) 
		window.scrollTo(0,document.body.scrollHeight); 
     }
     function initWebSocket() {
         websocket = new WebSocket(gateway);
         websocket.onopen = onOpen;
         websocket.onclose = onClose;
         websocket.onmessage = onMessage;
     }
     function htmlenc(str) {
         const map = { '<': '&lt;', '>': '&gt;', '&': '&amp;' };
         return str.replace(/<>&/,function(match) { return map[match]; });
     }
</script>
	<body>
		<div id=log></div>
	</body>
</html>
)";


