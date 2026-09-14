# LabAnalyser-Vertrag fuer den FPGA-Streamer

Stand der Sichtung: 2026-09-12

## Ergebnis

LabAnalyser selbst benoetigt fuer den ersten Ausbau keine neue Plugin-API. Ein
separates Qt-Plugin kann ueber den bestehenden `Platform_Fabric`-Vertrag geladen
werden, Signale publizieren und Zeitreihen als `DataPair` liefern. Das minimiert
das Risiko fuer bestehende Plugins und alte `.LAexp`/`.LAdev`-Dateien.

## Vorgeschlagene `.LAdev`

```xml
<?xml version="1.0" encoding="UTF-8"?>
<LEDevice DevicePlugin="StreamerPlugin.dll" DeviceName="FpgaTrace">
  <Connection IP="10.0.1.155" Device="1" Core="0" />
  <StreamDefinition File="fpga_trace_signals.xml" />
</LEDevice>
```

Der Verweis ist relativ zum Ordner der `.LAdev`; absolute Pfade koennen zur
Kompatibilitaet zusaetzlich akzeptiert werden. Die generische LabAnalyser-
Laderoutine ignoriert unbekannte Kindknoten. Das neue Plugin bekommt danach den
Pfad der `.LAdev` im bestehenden `load`-Kommando und liest den Verweis selbst.
Damit ist keine Aenderung am zentralen Experiment-Reader noetig.

## Vorgeschlagenes Signal-XML

```xml
<?xml version="1.0" encoding="UTF-8"?>
<FixedPointStream schemaVersion="1"
                  byteOrder="little"
                  bitOrder="lsb0"
                  blockBits="64"
                  samplePeriodSeconds="0.000001">
  <Signal name="signal1"
          bitOffset="0"
          bitWidth="18"
          signed="true"
          scale="0.00123454"
          unit="A" />
  <Signal name="signal2"
          bitOffset="18"
          bitWidth="14"
          signed="false"
          scale="0.01"
          unit="V" />
  <Signal name="enabled"
          bitOffset="32"
          bitWidth="1"
          type="bool" />
</FixedPointStream>
```

Verbindlicher Interpretationsvertrag:

```text
unsigned: raw = extrahierte Bits
signed:   raw = Zweierkomplement mit bitWidth Bits
value:    double(raw) * scale
```

`bitOffset` wird ab dem niederwertigsten Bit des ersten Bytes gezaehlt. Namen
muessen eindeutig sein, Bereiche duerfen nicht ueberlappen und
`bitOffset + bitWidth` muss kleiner oder gleich `blockBits` sein. `bitWidth`
liegt zwischen 1 und 64. `scale` wird locale-unabhaengig als endlicher positiver
oder negativer Double-Wert gelesen; Null sollte als Konfigurationsfehler gelten.
`samplePeriodSeconds` definiert die Zeitachse, wenn die akzeptierten
`valid_in && ready_out`-Bloecke periodisch sind. Bei nicht periodischer
Gueltigkeit muss ein Sample-Counter oder Zeitstempel Bestandteil von `data_in`
sein und wie ein normales unsigned Signal beschrieben werden. Die FIFO-
Elementgroesse ist eine Transporteigenschaft und wird bewusst nicht in diesem
Signal-XML dupliziert.

Ein einzelnes FPGA-Bit wird explizit mit `type="bool"` und `bitWidth="1"`
beschrieben. Bei Bool-Signalen sind `signed` und `scale` nicht erlaubt. Der
aktuelle Wert wird als nativer LabAnalyser-`bool` publiziert; seine Zeitreihe
bleibt wegen des bestehenden `DataPair`-ABI als 0.0/1.0 codiert.

## Publizierte IDs

Empfohlen werden pro Signal zwei Datenobjekte:

```text
FpgaTrace::Signals::signal1             letzter skalierter Wert, double
FpgaTrace::Buffered::Signals::signal1   Zeitreihe, vector<double>/DataPair
FpgaTrace::Signals::enabled             letzter Wert, bool
FpgaTrace::Buffered::Signals::enabled   Zeitreihe, 0.0/1.0 als DataPair
```

Zusaetzliche Software-Statuswerte sollten mindestens empfangene Bloecke,
ungueltige Elementlaengen und Verbindungszustand sichtbar machen. Ein FPGA-
Overflow kann nur angezeigt werden, wenn das einfache Modul dafuer optional
einen separaten Statusausgang erhaelt; im rohen Datenstrom steht kein Statusfeld.

Das StreamerPlugin startet hoechstens zehn UltraTracer-Abfragen pro Sekunde.
Zwischen zwei Request-Startzeitpunkten liegen mindestens 100 ms, auch nach
leeren oder ungueltigen Antworten. Die Linux-FIFO-Tiefe muss deshalb passend
zur Samplefrequenz dimensioniert werden; sie bleibt eine Transporteigenschaft
und ist kein Bestandteil des Signal-XML.

## Kompatibilitaet und Tests

- Keine Aenderung an `platforminterface.h`, `InterfaceDataType` oder der IID.
- Bestehende `.LAdev` ohne `StreamDefinition` bleiben unveraendert gueltig.
- Fehlende oder ungueltige Signal-XML verhindert die Stream-Aktivierung mit
  einer klaren Fehlermeldung, aber ohne Host-Absturz.
- Der vorhandene Plugin-Contract-Test wird um eine kompatible Streamer-Fixture
  und den `.LAdev`-Verweis erweitert.
- Parser und Decoder erhalten eigenstaendige Tests ohne GUI und Netzwerk.
- Jede empfangene Payload muss ein ganzzahliges Vielfaches von
  `ceil(blockBits / 8)` sein; andernfalls wird sie vollstaendig verworfen.

Die Sichtung war dokumentarisch. Es wurden keine LabAnalyser-Produktionsdateien
oder ABI-Grenzen geaendert und deshalb gemaess Repository-Regel keine Builds
gestartet.
