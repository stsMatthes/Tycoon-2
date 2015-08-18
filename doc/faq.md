 

### Tycoon-2 FAQ (Frequently Asked Questions)

**Stand**: 4.11.1997 (FM) 13.01.1998 (Anpassungen JW)

Bitte weitere Fragen und Vorschläge für Antworten an wahlen@tu-harburg.de schicken!

#### Was sind Metaklassen?

Jedes Objekt gehoert zu einer Klasse. Auch Klassen sind Objekte.
Die Klassen, zu denen sie gehoeren, nennt man Metaklassen.  Metaklassen
bieten die Methoden, um Instanzen ihrer Klasse zu erzeugen.  Im
Normalfall (bei ConcreteClass und deren Nachfahren) gibt es ein
"new"-Methode, die ein neues Exemplar liefert.  Weitere wichtige
Metaklassen sind "MetaClass" (die Metaklasse der Metaklassen) und
"AbstractClass" (fuer Klassen, die nicht instantiiert werden sollen).

Wenn man nicht explizit mittels "metaclass ..." eine Metaklasse
angibt, wird der Namen der Klasse mit einem angehaengten "Class"
benutzt.  Die Metaklasse zu Reader heisst demnach ReaderClass etc.

Fuer einfache Faelle gibt es SimpleConcreteClass, das ein generisches
new ohne Argumente zur Verfuegung stellt.

Sollen bei der Erzeugung einige Slots schon mit sinnvollen Werten
initialisiert werden, definiert man in der Metaklasse eine new-Methode
mit spezifischen Argumenten. Diese new-Methode legt mittels "\_new"
(geerbt von ConcreteClass) eine Instanz an, weist einige Slots zu und
gibt die erzeugte Instanz an den Klienten weiter.

#### Wie bringe ich Tycoon-2 dazu, bei Set.new den Typ des zu erzeugenden Sets zu inferieren?

Die Zauberformel heisst: Set(:Int).new

Nicht etwa: Set(Int).new

Das liegt daran, dass der Name der Klasse eine Bezeichner auf der Wertebene
ist. "Set" ist implizit eine Methode der Klasse Object, die einen Typ-
Parameter nimmt. Und wie immer bei Methodenaufrufen muss vor Typ-Parametern
ein Doppelpunkt stehen.

Wird "Set" auf der Typebene benutzt, z.B.:   "intset :Set(Int)", dann
fungiert es als Typoperator. Dessen Args werden nicht gedoppelpunktet.

**Wie erlaube ich nur lese-Zugriff auf Slots?**

Antwort: Den Slot private unter einem anderen Namen (z.B. mit
underscore '\_') definieren und eine Zugriffsmethode schreiben:

  public methods
  value :Int
  { \_value }

  private
  \_value :Int

Oder alternativ (weniger fein): Den Slot privat deklarieren, und die
Zugriffsmethode mit gleichem Namen public.  Die Zugriffsmethode kann dann
mittels super.&lt;slotname&gt; auf den Slot zugreifen:

  public methods
  value :Int
  { super.value }
  private
  value :Int

**Vorsicht Falle: Bei Methoden wird der Ergebnistyp im Gegensatz zu**
**anonymen Funkionen nicht inferiert, sondern als Void angenommen.**

Insbesondere in new-Methoden von Klassen vergisst man leicht, den
Ergebnistyp hinzuschreiben, was zu Fehlern fuehrt, wenn man dann
tatsaechlich so ein Objekt erzeugt und benutzen will.

**Steckt ein Trick hinter der komischen if-Syntax?**

Ja. Die Syntax

        &lt;test&gt; ? &lt;ifTrue&gt;
bzw.
        &lt;test&gt; ? &lt;ifTrue&gt; : &lt;ifFalse&gt;

wird intern uebersetzt ("normalisiert") in einen Methodenaufruf:

        &lt;test&gt;."?"(&lt;ifTrue&gt;)
bzw.
        &lt;test&gt;."?:"(&lt;ifTrue&gt;, &lt;ifFalse&gt;)

&lt;ifTrue&gt; und &lt;ifFalse&gt; sind dabei Funktionen ohne Argumente (aka
Bloecke, Fun0s). Je nachdem, ob &lt;test&gt; zur Klasse True oder False
gehoert (d.h. true oder false ist), wird einer der beiden aufgerufen.

&lt;ifTrue&gt; und &lt;ifFalse&gt; muessen Bloecke und keine einfachen Ausdruecke
sein, da sonst immer beide Zweige ausgewertet wuerden (man simuliert
lazy evaluation).

Aehnlich funktioniert die Mechanik hinter "||" und "&&", genaueres
verraet der Quellcode (Klassen Bool, True, False).

**Wie stelle ich "Variante Records" dar?**

Auf jeden Fall werden alle Varianten als Subklassen eines gemeinsamen
Vorfahren implementiert.  Nehmen wir folgenden Modula-Record:

  TYPE A =
         RECORD
           i: INTEGER;
           CASE whichOne: \[1..3\] OF
             1: c :CHAR;
           | 2: b :BOOLEAN;
           | 3: s :ARRAY OF CHAR;
         END;

In Klassen saehe das so aus:
class A public i :Int;
class A1 super A public c :Char;
class A2 super A public b :Boolean;
class A3 super A public s :String;

Die Frage ist nun, wie man auf die verschiedenen Varianten zugreift,
d.h. wie man herausfindet, mir welcher Variante man es konkret zu tun
hat.

Es gibt vier verschiedene Ansaetze: den rein objektorientierten,
Implementation mittels case-Methode, mittels Visitor, und mittels
dynamischem Klassen-Test.

- rein objektorientierter Ansatz

Bei konsequent objektorientierter Implementation wird das gesamte
Verhalten ins Objekt gesteckt, d.h. fuer jede moegliche Verwendung des
Objekts gibt es eine eigene Methode.  Diese Methode wird in der
Vorfahrenklasse "deferred" oder mit einem default-Verhalten
deklariert, und die Subklassen koennen sie dann redefinieren.  Die
Typ-Unterscheidung wird also durch Message-Dispatch ausgefuehrt.

  Der Nachteil dieser Methode ist, dass das Verhalten sehr stark
verteilt wird: in jeder Klasse steht ein bisschen Code.  Es gibt
Faelle, in denen der Zusammenhalt des Algorithmus wichtiger ist als
der Zusammenhalt des Objekts.

  In solchen Faellen kann man eine explizite case-Methode benutzen.

- case

Erstmal eine ganz abstrakte Erklaerung: Die case-Methode bekommt einen
Block fuer jede moegliche Variante uebergeben.  Es wird dann der zum
Typ des Empfaengers passende Block aufgerufen.  Um auf spezifische
Felder der Subklassen zugreifen zu koennen, muss man den genauen Typ
kennen; zu diesem Zweck uebergibt der Empfaenger sich selbst mit
bekanntem Typ an den jeweiligen Block.
  An unserem Beispiel:

class A
public
  i :Int
methods
case(T &lt;: Void,
     a1 :Fun1(A1,T),
     a2 :Fun1(A2,T),
     a3 :Fun1(A3,T)) :T
  deferred
;

class A1
super A
public
  c :Char
methods
case(T &lt;: Void,
     a1 :Fun1(A1,T),
     a2 :Fun1(A2,T),
     a3 :Fun1(A3,T)) :T
{
  a1\[self\]
}
;

Und so weiter fuer die anderen Klassen.  Benutzt wird das z.B. so:

m(a :A)
{
  a.case(
    fun(a1 :A1) {
      "Character " + a1.c.printString
    },
    fun(a2 :A2) {
      "Boolean " + (a2.b ? {"Ja"} : {"Nein"})
    },
    fun(a3 :A3) {
      "String " + a3.s
    }
  ).printOn(tycoon.stdout)
}

Siehe auch Klassen Bool, TWL/Compile/Parse/LALRAction,
TWL/Compile/Parse/Symbol, TWL/Compile/TWL/Ide.

Wie man schon sieht, legt man sich mit diesem Muster auf eine feste Anzahl
Varianten fest, was beim OO-Ansatz nicht der Fall ist.  Will man nachtraeglich
eine Variante hinzufuegen oder aendern, muessen die abstrakte Oberklasse und
alle Varianten angefasst werden.

- visitor

Wird die Anzahl der zu unterscheidenden Faelle zu gross, greift man
zum Visitor.  Der Visitor ist ein Objekt, das alle oben einzeln
uebergebenen Bloecke als Methoden enthaelt und zusammenfasst.  Fuer
jeden moeglichen "Gastgeber" gibt es also eine eigene Methode, die
konventionell durch "visit&lt;Klassenname&gt;" gebildet wird. Beispiel:

class A
public
  i :Int
methods
visit(T &lt;: Void, visitor: AVisitor(T)) :T
  deferred
;

class A1
super A
public
  c :Char
methods
visit(T &lt;: Void, visitor: AVisitor(T)) :T
{
  visitor.visitA1(self)
}
;

class AVisitor(T &lt;: Void)
public methods
visitA1(a1 :A1) :T
  deferred
visitA2(a2 :A2) :T
  deferred
visitA3(a3 :A3) :T
  deferred
;

Um ein konkretes Verhalten zu implementieren, muss man eine Subklasse von
AVisitor erzeugen, z.B.:

class DummyAVisitor
super AVisitor(String)
metaclass SimpleConcreteClass(DummyAVisitor)
public methods
visitA1(a1 :A1) :String
{ "Character " + a1.c.printString
}
visitA2(a2 :A2) :String
{ "Boolean " + (a2.b ? {"Ja"} : {"Nein"})
}
visitA3(a3 :A3) :String
{ "String " + a3.s
}
;

...und benutzt wird das so:

m(a :A)
{
  a.visit(DummyAVisitor.new).printOn(tycoon.stdout)
}

Dieses Muster ist u.a. dann vorteilhaft, wenn der Visitor
weitergereicht werden soll (z.B. in Baeumen), da ein einzelnes Objekt
natuerlich griffiger ist als ein halbes Dutzend Codebloecke.
Ausserdem praktisch ist, dass der End-Benutzer (Methode m im obigen
Beispiel) nicht wissen muss, welche Varianten es gibt.

- dynamischer Klassen-Test

In Tycoon-2 ist es zur Zeit schon (in wird in Zukunft noch viel mehr)
moeglich, reflexiv zur Laufzeit auf Klassen und Metaklassen
zuzugreifen.  Mittels x."class" erhaelt man die Klasse, zu der x
gehoert, als Laufzeitobjekt.  Dieses Objekt kann man nach seinem Namen
fragen (x."class".name) oder es mit anderen Objekten vergleichen
(x."class" == Set).  Folgender Hack waere also denkbar:

m(a :A)
{
  let clazz = a."class",
  clazz == A1 ? {
    let a1 = \_typeCast(a, :A1),
    "Character " + a1.c.printString
  } : {
    clazz == A2 ? {
      let a2 = \_typeCast(a, :A2),
      "Boolean " + (a2.b ? {"Ja"} : {"Nein"})
    } : {
      clazz == A3 ? {
        let a3 = \_typeCast(a, :A3),
        "String " + a3.s
      } : {
        SomeError.new.raise
      }
    }
  }.printOn(tycoon.stdout)
}

Dies ist *KEIN EMPFOHLENER PROGRAMMIERSTIL*.  Ein wichtiger Nachteil
ist, dass fuer Subklassen von z.B. A1 nicht mehr "class == A1" gilt;
der Code ist also noch staerker als beim case/visitor-Ansatz vom
aktuellen Klassen-Schema abhaengig.

#### Was ist ein Typoperator, und wozu brauche ich ihn?

Ein Typoperator ist eine Funktion auf der Typebene: Man uebergibt
einige Typen als Argument, und das Ergebnis ist ein neuer Typ.  In Tycoon-2
definiert man Typoperatoren, indem man Klassen mit Argumenten
definiert:

class Array(E &lt;: Object)
(\*...\*)
public methods
"\[\]"(i :Int) :E
;

Wenn ich jetzt einen bestimmten Arraytyp meine, muss ich den aktuellen
Wert fuer E angeben. Das sieht so aus:

x :Array(Char)

Dabei fungiert "Array" als Tyoperator, dem ich "Char" uebergebe.
Daraufhin werden alle Es in Array durch Char ersetzt, und ich erhalte
einen Typ, der u.a. die Methode

  "\[\]"(i :Int) :Char

enthaelt.  Der Typ Array heisst "offen", weil der Parameter E offen
bleibt; Array(Int) heisst dementsprechend "geschlossen".  Nur fuer
geschlossene Typen kann man Werte angeben.

#### Unter welchen Annahmen ueber die Signatur von Self wird eine Klasse getypecheckt ? Muss immer gelten Self &lt;: C ? Ist eine explizite Angabe der Signatur von Self ein zusaetzlicher oder ein ausschliesslicher Hinweis ?

In einer Klasse "class A(B &lt;: C)" ist Self implizit deklariert als
"Self &lt;: A(B)".  Gibt man einen anderen bound fuer Self an, so muss
die Signatur der Klasse dazu passen.  Fuer interessante Verwendungen
von Self siehe Number und dessen Subklassen.  Will man die Signatur
einer Klasse irgendwann einfrieren, kann man sie (im obigen Beispiel)
als "Self = A(B)" deklarieren.  Diese Einschraenkung muessen dann auch
alle Subklassen erfuellen.

#### Was bedeutet eine Fehlermeldung der folgenden Form?

"&lt;&lt;
Checking class 'NonterminalClass'
../TWL/Compile/Parse/NonterminalClass.tc:1.1 Illegal inheritance (inconsistent class precedence list):
&gt;&gt;"

Die Fehlermeldung bedeutet, dass eine Methode inkompatibel redefiniert
wird.  Inkompatibel heisst wohl "die Signatur der Methode in der
Subklasse ist keine Subsignatur der Signatur in der Superklasse".

Das kann zum Beispiel daran liegen, dass man einen Typoperator statt
eines geschlossenen Typs als Argument an eine Vorfahrenklasse
uebergeben hat:
class A(T &lt;: Void) methods m(x :T) ...;
class B(E &lt;: Void) ... ;
class C super A(B) ...   \#\#\# Krach 

#### Pool-Variablen

Ich wuerde gerne Objekte interaktiv testen. Kann ich auf dem Toplevel
bindings deklarieren ?

let und Let sind am toplevel leider nicht moeglich.Fuer Typen muss man eine eigene Klasse erzeugen,
fuer Werte benutzt man define:

  define x :T;

Das definiert eine Pool-Variable mit Namen x und Typ T, die mit nil
initialisiert wird.  Pool-Variablen werden konzeptionell als Slots der
Klasse Object eingehaengt.  Dadurch kann man in jeder Klasse auf x wie
auf einen eigenen Slot zugreifen: Effektiv ist x eine globale
Variable.

  Ausdruecke am Toplevel werden im Kontext der Klasse Nil ausgewertet.
Da auch Nil Subklasse von Object ist, stehen einem die Pool-Variablen
also auch am Toplevel zur Verfuegung.

  Ueber den Pool-Mechanismus wird auch der Zugriff auf Klassen als
Objekte geregelt: Wenn ich am Toplevel "List.new" schreibe, wird das
uebersetzt ist "self.List.new".  Da es in der Klasse nil keinen Slot
"List" gibt, wird im Pool nachgeschaut, und siehe da, wir finden die
gesuchte Klasse.  Mal ausprobieren:

"tycoon.tl.pool.poolMethods.keys.do(fun(s :Symbol){tycoon.stdout &lt;&lt; s, tycoon.stdout.nl});"

Siehe auch Abschnitt "Set.new".

  Pool-Variablen haben zwei Nachteile: 1. Man wird sie nicht wieder
los (bzw. nur durch den reflektiven Aufruf tycoon.tl.pool.undefinePoolVariable("x")), und
2. sie koennen zu Konflikten mit existierenden Methoden oder Slots
fuehren.  Als Ausweg empfiehlt es sich, fuer adhoc-Variablen
moeglichst kryptische Namen zu waehlen - sowas kommt in unseren
super-designten Libraries naemlich nicht vor ;-)

  Um eine veraltete Klasse X loszuwerden, kann man sie mittels
"class X metaclass AbstractClass;" leer ueberschreiben.

#### Singletons

Es gibt Klassen, von denen man nur eine einzige Instanz im System
haben will (ein wichtiges Beispiel ist das Betriebssystem, Klasse
Tycoon).  Dies erreicht man, indem die new-Methode der Metaklasse neu
erzeugte Objekte einer Pool-Variable zuweist.  Der normale Benutzer
greift dann nur noch auf diese Pool-Variable zu, mit der new-Methode
hat er nichts mehr zu tun.

  Per Konvention heisst die Pool-Variable genauso wie ihre Klasse, nur
wird sie klein geschrieben (Klasse Tycoon, Variable tycoon).

#### Das System "vergisst" einige Elemente meines Arrays! Wo bleiben die?

Wenn der Array "nil" enthaelt, und man einen Reader fuer die Elemente
des Arrays anfordert, kann der Reader nicht zwischen nil als "end of
stream" und nil als Element des Objektstroms unterscheiden.  Der
Klient des Readers sieht daher nur die Elemente bis zum Auftauchen des
ersten nil und hoert auf, weil er glaubt, er sei am Ende angelangt.

#### Werden pre- und postconditions ueberhaupt zur Laufzeit getestet?

Nein.  Genausowenig wie invarianten.  Es ist aber sinnvoll, sie trotzdem
hinzuschreiben und so zu tun, als wuerden sie ueberprueft; das ist eine
Art von sehr exakten Kommentaren.
 

<table>
<colgroup>
<col width="50%" />
<col width="50%" />
</colgroup>
<tbody>
<tr class="odd">
<td align="left"></td>
<td align="left"><a href="/~f.matthes">f.matthes</a>, nov-1997 </td>
</tr>
</tbody>
</table>

 
