# Simple Clicker Hilfehandbuch

- Du kannst diese Anwendung für automatisierte Mausklicks verwenden.
- WARNUNG: Diese Software sollte nicht als Auto-Click-Cheat in Spielen verwendet werden. Falls du dies tust, sind wir als Entwickler dieser App ABSOLUT NICHT dafür verantwortlich. Lies die Lizenz bezüglich der Gewährleistung.

## Voreinstellungen

- Du kannst beliebig viele Voreinstellungen haben.
- Jede Voreinstellung hat ihr eigenes "Einstellungspaket".
- Du kannst eine für einen bestimmten Zweck und eine andere für einen anderen Zweck verwenden.
- Doppelklicke im linken Menü auf eine Voreinstellung, um zu ihr zu wechseln.

### Voreinstellung erstellen
- Erstelle eine neue Voreinstellung mit der Schaltfläche `+` rechts neben dem Titel `Voreinstellungen` oben links im Fenster.
- Nach dem Erstellen kannst du ihr direkt einen Namen geben, indem du ihn eintippst.
- Standardmäßig wird für jede erstellte Voreinstellung ein Name wie `Neue Voreinstellung N` generiert.
- Wenn du mit dem Umbenennen fertig bist, drücke `ENTER` oder die Speichern-Schaltfläche rechts neben dem Namensfeld der Voreinstellung, um den neuen Namen zu übernehmen.
- Du kannst das Umbenennen auch mit `ESC` oder der Abbrechen-Schaltfläche rechts neben der Speichern-Schaltfläche abbrechen.

### Voreinstellung speichern/verwerfen
- Wenn eine Einstellung auf der rechten Seite geändert wird, wird der Name der Voreinstellung kursiv und fett dargestellt und links vom Namen erscheint ein Sternchen. Das zeigt an, dass die Voreinstellung geändert wurde.
- Klicke auf das Speichern-Symbol oder drücke `Ctrl+S` auf der Tastatur, um die Änderungen zu speichern.
- Klicke auf das Abbrechen-Symbol oder drücke `Ctrl+K` auf der Tastatur, um die Änderungen zu verwerfen. (Setzt die Änderungen zurück)

### Voreinstellungen verwalten
- Um eine Voreinstellung zu löschen, bewege zuerst die Maus über die entsprechende Voreinstellung, dann erscheint eine Papierkorb-Schaltfläche. Klicke darauf, um die Voreinstellung zu löschen.
- Um eine Voreinstellung erneut umzubenennen, bewege zuerst die Maus über sie, dann erscheint ein Stift-Symbol. Klicke darauf, es wird ein Texteingabefeld angezeigt, in das du den gewünschten neuen Namen eingibst. Drücke dann `ENTER` oder klicke auf die Speichern-Schaltfläche, um das Umbenennen zu übernehmen, ODER drücke `ESC` oder klicke auf die Abbrechen-Schaltfläche, um das Umbenennen abzubrechen.
- Während des Umbenennens kannst du weiterhin die Einstellungen der Voreinstellung ändern. Wenn du das Umbenennen jedoch übernimmst, wird die Voreinstellung ebenfalls gespeichert. Dasselbe gilt für das Abbrechen des Umbenennens (verwirft die Änderungen).

## Einstellungen der Voreinstellung
- Du kannst die Einstellungen in der Oberfläche rechts neben den Voreinstellungen ändern.
- `Klickintervall` legt fest, wie viel Zeit zwischen den Klicks vergeht. Der Wert muss größer als 0 sein.
- `Haltedauer` legt fest, wie lange eine Maustaste gedrückt gehalten wird. Dieser Wert muss größer als 0 sein.
- `Klickoptionen` legt die Details der Mausklicks fest.
- `Maustaste` ist die Maustaste, die geklickt wird. Du kannst Links, Rechts, Mitte, Vor (Seiten) und Zurück (Seiten) wählen.
- `Klickmodus` legt fest, ob die Maustaste gedrückt und wieder losgelassen oder gehalten wird.
- `Wiederholen` legt fest, wie oft der Klick wiederholt wird. Der Wert muss größer als 0 sein. Das Limit liegt bei 1 Million.
- Wenn `Wiederholen, bis gestoppt wird` ausgewählt ist, wird so lange geklickt, bis du das Tastenkürzel zum Stoppen drückst.
- `Mauszeigerposition` ist die absolute Position auf deinem Monitor, an der ein Klick ausgeführt wird. Bei mehreren Monitoren funktioniert dies möglicherweise nicht.
- Wenn du `Aktuell` wählst, sind die X- und Y-Werte nicht relevant, da der Klick dort ausgeführt wird, wo sich dein Mauszeiger befindet.
- Um eine Position über die Oberfläche auszuwählen, klicke auf `Auswählen`. Der Bildschirm wird eingefroren und abgedunkelt, damit du eine Position wählen kannst.
- Bewege den Mauszeiger an die gewünschte Position und klicke mit der linken Maustaste, um diese Position auszuwählen, oder drücke `ESC`, um abzubrechen.

## Einstellungen
- Klicke unten im Fenster auf die Schaltfläche `Einstellungen`, um das Einstellungsfenster zu öffnen.
- Hier kannst du Design, Sprache und das globale Tastenkürzel ändern.
- Unterstützt Sprachen: Englisch, Türkisch, Deutsch und Französisch.
- Unterstützte Designs: Dunkel und Hell.
- Um das Tastenkürzel zu ändern, klicke auf das Textfeld und drücke eine Taste oder Tastenkombination auf deiner Tastatur.
- Klicke dann auf Speichern, um die Einstellungen zu speichern, oder auf Abbrechen, um Änderungen zu verwerfen.

## Tastenkürzel
- Das von dir festgelegte Tastenkürzel wird von der Anwendung überall abgehört, solange die Anwendung nicht geschlossen ist.
- Drücke das Tastenkürzel, um den Clicker ein- bzw. auszuschalten.
- Dieses Verhalten ist unter Linux mit dem Wayland-Compositor anders.
- Wayland beschränkt globale Tastenbelegungen auf das Programm selbst, dies gilt jedoch nicht für KDE Plasma.
- In KDE sind globale Tastenkürzel über XCB möglich, da das Programm unter XWayland läuft.
- Unter X11 funktionieren globale Tastenbelegungen einwandfrei, genau wie unter Windows.

## Automatisches Update
- Das Programm informiert dich beim Start, wenn ein neues Update verfügbar ist.
- Es erscheint eine Benachrichtigung wie `Neues Update verfügbar`. Klicke auf `Jetzt aktualisieren`, um die Anwendung zu aktualisieren.
- Dieser Update-Vorgang läuft vollautomatisch ab. Er fragt lediglich unter Windows nach Administratorrechten und unter Linux nach Root-Rechten.
- HINWEIS: Diese Berechtigungen sind zur Installation der Anwendung notwendig. Wenn du alle Installationsschritte und Vorgänge im Hintergrund nachvollziehen möchtest: Das Projekt ist Open Source.
- Die Anwendung wird nach Abschluss des Updates automatisch geöffnet.

# Über

- Entwickelt von [ilpeN](https://github.com/ilpenSE).
- In C++ mit Qt entwickelt.
- Lizenz: ausschließlich AGPL-3.0 (GNU Affero General Public License)
