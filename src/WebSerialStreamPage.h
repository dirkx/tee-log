#if 0
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
 	 tag.innerHTML += htmlenc(event.data) + "\n";
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
#else
// Compressed version.
static const char page[] PROGMEM = R"(<title>Log</title><style>#log{font-family:'Courier New',monospace;white-space:pre}</style><script>var ws,g="ws://"+location.host+"%s";function ol(e){initWebSocket()}function oO(e){ws.send("getHistory")}function oC(e){setTimeout(initWebSocket,2e3)}function oM(e){var n=window.innerHeight+window.pageYOffset>=document.body.offsetHeight-4;for(tag=document.getElementById("log");tag.innerHTML.length>5191680;)tag.innerHTML.slice(0,tag.innerHTML.indexOf("\n"));tag.innerHTML+=he(e.data)+"\n",n&&window.scrollTo(0,document.body.scrollHeight)}function initWebSocket(){(ws=new WebSocket(g)).onopen=oO,ws.onclose=oC,ws.onmessage=oM}function he(e){const n={"<":"&lt;",">":"&gt;","&":"&amp;"};return e.replace(/<>&/,(function(e){return n[e]}))}window.addEventListener("load",ol);</script><div id=log></div>)";
#endif
