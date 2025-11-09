In order to use this template of a server in java, download every file in this repo.

Steps to use it:
-> Compile in the terminal in the root directory, this will compile every .java files in GameServer :

  $files = Get-ChildItem -Path .\GameServer -Recurse -Filter *.java | ForEach-Object { $_.FullName }
  javac -d out $files

-> Excecute:
java -cp out GameServer.Core.TestingGameServer 12345 4

-> Test it on a client:
nc localhost 12345
JOIN|Player1
CHAT|Hola
PING|

  - You should see a debbug message like:
     ACCEPT|<id>, CHAT|<id>: Hi, PONG|.
