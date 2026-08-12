# MatlabRemoteConnector – Funktionsbeschreibung

## Zweck

`MatlabRemoteConnector` verbindet MATLAB mit der lokalen TCP-Schnittstelle von
LabAnalyser. MATLAB ruft dazu keine TCP-Funktionen direkt auf, sondern lädt die
native Windows-DLL `TCPClient.dll`. Die MATLAB-Paketfunktionen im Verzeichnis
`+LabAnalyser` bilden die benutzerseitige API. Die DLL baut die TCP-Verbindung
auf, erzeugt und liest das binäre LabAnalyser-Protokoll und hält für jeden Port
einen eigenen Verbindungszustand.

Der gesamte Kommunikationsweg ist:

```text
MATLAB-Aufruf
    -> +LabAnalyser/*.m
    -> TCPClient.dll (C-API aus TCPClient.h)
    -> TCP über 127.0.0.1
    -> LabAnalyser RemoteControlServer
    -> MessengerClass / DataManagement / Plugins und GUI
```

Die Schnittstelle ist ausschließlich für Verbindungen auf demselben Rechner
ausgelegt. Der native Client verwendet immer `127.0.0.1`.

## Verbindungsaufbau

```matlab
LabAnalyser.Connect();       % Standardport 4080
LabAnalyser.Connect(4081);   % expliziter Port
```

`Connect.m` lädt bei Bedarf `TCPClient.dll` über `loadlibrary`, wobei
`+LabAnalyser/TCPClient.h` die exportierten Funktionen beschreibt. Vor dem
Neuaufbau wird eine eventuell vorhandene Verbindung für denselben Port
geschlossen. Anschließend erzeugt die DLL einen synchronen Winsock-Client und
verbindet sich mit `127.0.0.1:<port>`. Der Verbindungsversuch hat eine Frist von ungefähr
zwei Sekunden. Liefert die DLL einen Fehler zurück, erzeugt MATLAB einen Fehler.

LabAnalyser startet seinen `RemoteControlServer` beim Erzeugen des
Hauptfensters. Der Server versucht zuerst Port 4080 und erhöht die Portnummer,
bis ein freier lokaler Port gefunden wurde. Deshalb stimmt der MATLAB-Standard
4080 nur dann, wenn LabAnalyser tatsächlich auf 4080 lauscht. Der in
LabAnalyser angezeigte Remote-Port muss andernfalls explizit an die
MATLAB-Funktionen übergeben werden.

Die Verbindung wird so beendet:

```matlab
LabAnalyser.Disconnect();
LabAnalyser.Disconnect(4081);
```

`Disconnect.m` schließt die Verbindung für den Port und entlädt danach die
DLL. Während einer MATLAB-Sitzung sollte daher nach `Disconnect` vor weiteren
Zugriffen wieder `LabAnalyser.Connect(...)` aufgerufen werden.

## MATLAB-API

### Wert lesen

```matlab
y = LabAnalyser.Get("Device::Value");
[y, x] = LabAnalyser.Get("Device::Measurement");
allData = LabAnalyser.Get("*");
bufferData = LabAnalyser.Get("*Buffer*");
```

`Get` ruft intern `ExReceive` auf. Dieses sendet ein `get`-Telegramm mit der
angegebenen LabAnalyser-ID und wartet synchron bis zu zwei Sekunden auf eine
vollständige Antwort.

Die Rückgabe wird abhängig von der Serverantwort interpretiert:

- Ein numerischer Einzelwert wird als MATLAB-`double` in `y` zurückgegeben;
  `x` ist leer.
- Eine Zeichenkette wird in `y` zurückgegeben; `x` ist leer.
- Ein Messdatenpaar wird vom Server als `[Zeitwerte, Messwerte]` übertragen.
  `Get.m` teilt einen numerischen Vektor exakt in der Mitte: Die erste Hälfte
  wird `x`, die zweite Hälfte wird `y`.
- Bei einer leeren oder nicht gefundenen Antwort bleiben die Rückgaben leer.

Eine ID mit `*` ist eine Wildcard-Abfrage. `*` steht für beliebig viele
Zeichen; der Vergleich ist vollständig und unterscheidet Groß-/Kleinschreibung.
`Get("*")` liest somit alle aktuell im LabAnalyser-DataManagement vorhandenen
IDs, `Get("*Buffer*")` nur IDs, welche `Buffer` enthalten. MATLAB fragt zuerst
die sortierte Trefferliste ab und liest anschließend jeden Treffer mit seiner
exakten ID. Das Ergebnis ist eine `containers.Map`, deren Schlüssel die
vollständigen IDs sind. Jeder Wert ist ein Struct mit den Feldern `x` und `y`:

```matlab
result = LabAnalyser.Get("*Buffer*");
ids = keys(result);
first = result(ids{1});
plot(first.x, first.y);
```

Bei Einzelwerten und Text ist `x` leer. Eine Wildcard ohne Treffer liefert eine
leere `containers.Map`; der zweite Rückgabewert von `Get` ist dabei leer. Bricht
die Verbindung während der Trefferliste oder eines Einzelabrufs ab, meldet
MATLAB `LabAnalyser:ConnectionLost`. Danach ist ein erneutes `Connect` nötig.

Existiert eine ID ohne `*` nicht exakt, bleibt die bisherige Teil-ID-Suche
erhalten: LabAnalyser sucht alle IDs, welche den angefragten Text enthalten,
und gibt die Treffer als `|`-getrennte Zeichenkette zurück. Diese alte Suche
führt keinen automatischen Mehrfachabruf aus.

### Wert schreiben

```matlab
LabAnalyser.Set("Device::Gain", 2.5);
LabAnalyser.Set("Device::Mode", "automatic");
```

`Set.m` wählt den Übertragungsweg allein anhand des MATLAB-Typs:

- Ein MATLAB-Wert der Klasse `double` wird durch `ExSendDouble` gesendet.
- Jeder andere Wert wird durch `ExSendString` als C-Zeichenkette gesendet.

Der numerische DLL-Aufruf überträgt derzeit immer genau die ersten acht Byte,
also genau einen `double`. Vektoren können mit `Set` daher nicht als Vektor
geschrieben werden. Auf LabAnalyser-Seite wird der empfangene `double` in den
bereits vorhandenen Zieltyp konvertiert, beispielsweise `float`, Ganzzahl oder
`bool`.

Text wird NUL-terminiert übertragen und von LabAnalyser als Latin-1 gelesen.
Für `QStringList` ersetzt die Schnittstelle den Inhalt durch eine Liste mit
genau einem Element. Bei einer `GuiSelection` wird der Wert nur übernommen,
wenn er in der bestehenden Auswahlliste enthalten ist.

Ein `set` besitzt keine TCP-Antwort und keine Bestätigung. Der Server gibt die
Änderung über seinen `MessageSender` an den LabAnalyser-Messenger weiter. Dort
wird der DataManagement-Eintrag aktualisiert, und die Nachricht wird an
registrierte Plugins beziehungsweise GUI-Empfänger weitergeleitet.

## Binäres TCP-Protokoll

Alle Mehrbytewerte verwenden die native Byte-Reihenfolge und die nativen
Darstellungen der aktuellen Windows-Systeme. Das Protokoll enthält weder eine
Netzwerk-Byteordnung noch eine Versionskennung.

### Anfrage von MATLAB an LabAnalyser

```text
Offset  Größe                 Inhalt
0       4                     totalSize, uint32
4       3                     ASCII-Befehl "get" oder "set"
7       4                     idLength, uint32
11      4                     payloadLength, uint32
15      idLength              ID einschließlich abschließendem NUL
...     payloadLength         Nutzdaten
```

Dabei gilt:

```text
totalSize = 15 + idLength + payloadLength
```

Ein `get` hat keine Nutzdaten. Ein numerisches `set` enthält einen nativen
IEEE-754-`double` mit acht Byte. Ein Text-`set` enthält die C-Zeichenkette
einschließlich abschließendem NUL.

### Antwort von LabAnalyser auf `get`

```text
Offset  Größe                 Inhalt
0       1                     Typ: 0 = numerisch, 1 = Text
1       4                     elements, uint32
5       elements * 8          Datenbereich
```

Für numerische Antworten enthält der Datenbereich `elements` native
`double`-Werte. Einzelne numerische LabAnalyser-Typen werden für die Antwort in
einen `double` umgewandelt. Bei Messdaten werden zuerst alle Zeitwerte und
danach alle Messwerte übertragen.

Bei Text bezeichnet `elements` nicht nur die Zeichenanzahl, sondern die
Zeichenanzahl einschließlich NUL. Trotzdem reserviert das bestehende Protokoll
für jedes Element acht Byte. Der Text steht am Anfang dieses Bereichs, der Rest
ist mit Nullen aufgefüllt. `QStringList` liefert nur das erste Listenelement;
`GuiSelection` liefert nur den aktuell ausgewählten Text.

Eine leere beziehungsweise nicht unterstützte Antwort besteht aus Typ 0 und
`elements == 0` ohne Datenbereich.

## Zuständigkeiten der Dateien

- `+LabAnalyser/Connect.m` lädt die DLL und baut eine Verbindung auf.
- `+LabAnalyser/Disconnect.m` trennt die Verbindung und entlädt die DLL.
- `+LabAnalyser/Get.m` stellt die komfortable Lese-API bereit und trennt
  Messdaten in `x` und `y`.
- `+LabAnalyser/Set.m` wählt zwischen numerischem und textuellem Schreiben.
- `+LabAnalyser/ExReceive.m` führt den synchronen `get`-Ablauf über die DLL aus.
- `+LabAnalyser/ExSendDouble.m` bereitet einen numerischen Wert für die DLL vor.
- `+LabAnalyser/ExSendString.m` bereitet Text für die DLL vor.
- `+LabAnalyser/TCPClient.h` beschreibt MATLAB die exportierte DLL-Schnittstelle.
- `TCPClient.cpp` implementiert Telegramme, Empfangspuffer und die exportierte
  C-API.
- `TCPDummyClass.h` kapselt pro Port den Besitz des TCP-Clients.
- `TCPClientBare.cpp` implementiert den eigentlichen Winsock-Socket mit RAII.

## Bauen und testen

Der Connector kann unabhängig von LabAnalyser mit CMake gebaut und getestet
werden. Für den dokumentierten MSYS2-MINGW64-Compiler gilt beispielsweise:

```powershell
cmake -S MatlabRemoteConnector -B build/matlab-connector -G "MinGW Makefiles" `
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build/matlab-connector --parallel 2
ctest --test-dir build/matlab-connector --output-on-failure
```

Die installierbare MATLAB-Paketstruktur wird so erzeugt:

```powershell
cmake --install build/matlab-connector --prefix dist/LabAnalyser-release
```

Das Ergebnis liegt anschließend unter
`dist/LabAnalyser-release/+LabAnalyser`. Die MinGW-Laufzeit wird statisch in
`TCPClient.dll` gebunden; die DLL benötigt nur Windows-Systembibliotheken.

Der normale Root-Build `scripts/build-msys2.ps1` baut und testet den Connector
ebenfalls. Mit `-Deploy` installiert er das MATLAB-Paket unter
`<DeployDir>/+LabAnalyser`. Im GitHub-Artefakt liegt das Paket dadurch direkt
unter `./+LabAnalyser`, ohne zusätzlichen Zwischenordner.

## Aktuelle Einschränkungen und Risiken

- Die DLL wird nicht als Binärdatei versioniert, sondern reproduzierbar mit
  CMake gebaut und in das Windows-Release installiert.
- Port 4080 ist in MATLAB nur ein Standardwert. LabAnalyser kann auf einen
  höheren Port ausweichen.
- Lesen und Schreiben sind synchron und besitzen eine absolute Frist von zwei
  Sekunden. Bei Verbindungsabbruch, ausbleibender oder unvollständiger Antwort
  liefert der Leseaufruf keine Elemente und verwirft die betroffene Verbindung.
  Vor einem weiteren Zugriff muss `LabAnalyser.Connect(...)` aufgerufen werden.
- `Set` bestätigt weder Erfolg noch eine unbekannte ID. Eine Kontrolle ist nur
  durch anschließendes `Get` möglich.
- Numerisches Schreiben unterstützt nur einen Skalar; numerisches Lesen liefert
  grundsätzlich MATLAB-`double`.
- Das Protokoll ist plattformabhängig (native `uint32`-/`double`-Darstellung)
  und nicht versioniert.
- Die DLL serialisiert Zugriffe auf ihre nach Port getrennten Zustände. Ein
  blockierender Leseaufruf blockiert dadurch auch weitere DLL-Aufrufe, bis die
  Antwort eintrifft.
- LabAnalyser verarbeitet bei mehreren gleichzeitig verbundenen Clients nur
  den zuletzt akzeptierten Socket als aktuelle Verbindung.
- Antworten sind serverseitig auf insgesamt 1 MiB begrenzt. Wird diese Grenze
  überschritten, bricht LabAnalyser die aktuelle Verbindung ab.
- Wildcard-Treffer werden intern als `|`-getrennte ID-Liste übertragen. IDs
  mit einem literalen `|` sind deshalb für den Mehrfachabruf nicht eindeutig.

Der native DLL-Vertrag ist durch einen lokalen Loopback-Test gegen die
öffentliche C-API abgedeckt. Eine Ausführung innerhalb einer echten
MATLAB-Installation ist in der aktuellen Umgebung weiterhin nicht verifiziert.
