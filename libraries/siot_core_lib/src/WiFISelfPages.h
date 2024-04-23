/// @brief The homepage of SIOT_CORE_BOARD with function buttons
const char PROGMEM HOME_HTML[] = R"rawliteral(<!DOCTYPE html>
<html>
<head>
<title>SIOT - Homepage</title> 
<meta charset="utf-8">
<style>
html {
    font-size: 42px;
    padding-left: 5px;
    font-family: Arial, sans-serif;
}

.button {
  border: none;
  color: white;
  padding: 16px 32px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 2em;
  margin: 0.1em 15% 0.1em 15%;
  transition-duration: 0.4s;
  width: 70%;
}

.button1 {
  background-color: white; 
  color: black; 
  border: 2px solid #4CAF50;
}
.button1:hover {
  background-color: #4CAF50;
  color: white;
}
.button2 {
  background-color: white; 
  color: black; 
  border: 2px solid #008CBA;
}
.button2:hover {
  background-color: #008CBA;
  color: white;
}
</style>
<script>
    function GetSettings() {
        fetch('/cgi/settings')
        .then(res => res.text())
        .then(text => {
            let fields = text.split(',');
            document.getElementById("ssid").innerHTML = fields[0];
            document.getElementById("did").innerHTML = fields[2];
        })
    }
    function SetDeviceID() {
      let DeviceID = prompt("Please enter new Device ID:", "");
      if (DeviceID == null)  return;
      fetch('/cgi/save?d=' + DeviceID)
      .then(res => res.text())
      .then(text => {
        window.alert(text);
      })      
    }    
    function PureSettings() {
      if (confirm("Make sure that you want to clear all settings here?. Others should be kept.", "")) {
        fetch('/cgi/save?clear=true')
        .then(res => res.text())
        .then(text => {
        window.alert(text);
        });
      }
    }
</script>
</head>
<title>SIOT - Scan WiFi Networks</title> 
<body onload="GetSettings()">
<div>
	<h2>SIOT WiFi Adhoc</h2>
	<p>Detect wifi network and permanent store ssid/password inside the device</p>
</div>
<div>
	<div>Current WiFi SSID:
        <span style="color: green; font-weight: bold;" id="ssid"></span>
    </div>       
    <div>Device ID:
        <span style="color: purple; font-weight: bold;" id="did"></span>
    </div> 
</div>
<div>
<button class="button button1" onclick="window.location.href='/enroll';">WiFiScan</button>
<button class="button button2" onclick="SetDeviceID()">DeviceID</button>
<button class="button button2" onclick="PureSettings()">Pure settings</button>
<button class="button button2" onclick="window.location.href='/restart';">Restart</button>
</div>
</body>
</html>)rawliteral";

/// @brief The scanning wifi page of SIOT_CORE_BOARD with function buttons
const char PROGMEM ENROLL_HTML[] = R"rawliteral(<!DOCTYPE html>
<html>
<head>
<title>SIOT - Scan WiFi</title>    
<style>
  html {
      font-size: 42px;
      padding-left: 5px;
  }
 .button {
  border: none;
  color: white;
  padding: 10px 16px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 1.5em;
  margin: 0.1em 15% 0.1em 15%;
  transition-duration: 0.4s;
  width: 70%;
}
.button1 {
  background-color: white; 
  color: black; 
  border: 2px solid #4CAF50;
}
.button1:hover {
  background-color: #4CAF50;
  color: white;
}
.button2 {
  background-color: white; 
  color: black; 
  border: 2px solid #008CBA;
}
.button2:hover {
  background-color: #008CBA;
  color: white;
}
table {
  border-collapse: collapse;
  width: 100%;
}

th, td {
  text-align: left;
  padding: 8px;
}

tr:nth-child(even) {
  background-color: #D6EEEE;
}
</style>
<script>
function scanwifi(){
    var table = document.getElementById("WiFiList");
    var num = table.rows.length
    while (num > 1){
      table.deleteRow(1)
      num --;
    }
    fetch('/cgi/scan')
    .then(res => res.text())
    .then(text => {
      var list = text.split(',');
      list.push("&lt;hidden&gt;");
      list.push("")
      for (var i = 0; i < list.length; i++) {
          var row = table.insertRow((i>>1)+1);
          var cell1 = row.insertCell(0);
          var cell2 = row.insertCell(1);  
          var cell3 = row.insertCell(2);
          cell1.innerHTML = list[i]
          cell2.innerHTML = list[i+1]
          cell3.innerHTML = '<button class="button button2" onclick="setpassword(\''+list[i]+'\')">Save..</button>';
          i = i + 1;        
      }
    })
    .catch(err => { throw err });


}
function setpassword(ssid){
  if (ssid === "<hidden>") {
    ssid = prompt("Please enter the hidden WiFi Name:", "");  
    if (ssid == null)  return;
  }
  var pass = prompt("Please enter the password of the WiFi " + ssid +":", "");  
  if (pass == null)  return;
  //window.location.href='/cgi/save?s='+ssid+"&p="+pass;
  fetch('/cgi/save?s=' +ssid+ '&p=' +pass)
    .then(res => res.text())
    .then(text => {
      window.alert(text);
    }
  )
}
</script>
</head>
<body onload="scanwifi()">
<div>
  <a href='/'>&lt;&lt;&lt; </a>
  <h2>WiFi Networks</h2>
</div>
<p>select an WiFi SSID and save its password.</p>
<button class="button button1" onclick="scanwifi()">Rescan...</button>
<table id="WiFiList">
  <tr>
  <th>SSID</th><th>S/N</th>
  <th>Action</th>
  </tr>
</table>
</body>
</html>)rawliteral";