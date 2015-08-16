import java.awt.*;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.BitSet;
import java.util.StringTokenizer;
import java.util.Vector;
import java.applet.Applet;
import java.net.URL;

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* modelling tycoon events */

class TycObject {
TycObject(int Oid, String name)
{
  this.Oid = Oid;
  this.name = name;
}
TycObject(String info)
{
  int n = 0;
  while(n < info.length() && Character.isDigit(info.charAt(n)))
    ++n;
  Oid = Integer.parseInt(info.substring(0,n));
  while(n < info.length() && Character.isSpace(info.charAt(n)))
    ++n;
  name = info.substring(n,info.length());
}
public String toString() {
  return name;
}
int Oid;
String name;
}

interface ObjectWriter {
  void write(char c);
  void write(String str);
  void write(TycObject obj);
}

class StringObjectWriter implements ObjectWriter {
  StringBuffer buf = new StringBuffer();
  public void write(char c) {
    buf.append(c);
  }
  public void write(String str) {
    buf.append(str);
  }
  public void write(TycObject obj) {
    buf.append((obj==null)?"nil":obj.name);
  }
  public String toString() {
    return buf.toString();
  }
}

interface TycEventVisitor {
void visitSend(TycSendEvent evt);
void visitReturn(TycReturnEvent evt);
void visitException(TycExceptionEvent evt);
}

abstract class TycEvent {
  int seqNo;
  TycObject thread;
  TycObject sender, sendingComponent;
  TycObject receiver, receivingComponent;
  abstract void accept(TycEventVisitor v);
  abstract void writeOn(ObjectWriter w);
  public String toString() {
    StringObjectWriter w = new StringObjectWriter();
    writeOn(w);
    return w.toString();
  }
  Activation sendingActivation, receivingActivation;
}

class TycSelector {
  TycSelector(String info) {
    int lastSlash = info.lastIndexOf('/', info.length()-1);
    arity = info.length()-1 - lastSlash;
    sorts = new BitSet(arity);
    int sortStart = lastSlash+1;
    for(int i=0;i<arity;++i) {
      if(info.charAt(sortStart+i) == 'C')
        sorts.set(i);
    }
    int butLastSlash = info.lastIndexOf('/', lastSlash-1);
    symbol = info.substring(0,butLastSlash);
  }
  String symbol;
  int arity;
  BitSet sorts;
  boolean returnsComponent()
  {
    return symbol.charAt(symbol.length() - 1) == '@';
  }
}

class TycSendEvent extends TycEvent {
    TycObject replyTo, replyToComponent;
    TycSelector selector;
    TycObject args[];
    
    void accept(TycEventVisitor v) {
	v.visitSend(this);
    }
    
    void writeOn(ObjectWriter w) {
	w.write(sender);
	w.write(": Send ");
	w.write(selector.symbol);
	w.write('(');
	for(int i=0; i<args.length; ++i) {
	    if(i>0)
		w.write(", ");
	    w.write(args[i]);
	    if(selector.sorts.get(i))
		w.write('@');
	}
	w.write(')');
	w.write(" to ");
	w.write(receiver);
    }
}

class TycReturnEvent extends TycEvent {
    TycObject result;
    boolean isComponent;

    void accept(TycEventVisitor v) {
	v.visitReturn(this);
    }
    
    void writeOn(ObjectWriter w) {
	w.write(sender);
	w.write(": Return ");
	w.write(result);
	if(isComponent)
	    w.write('@');
	w.write(" to ");
	w.write(receiver);
    }
}

class TycExceptionEvent extends TycEvent {
    TycObject exception;
    void accept(TycEventVisitor v) {
	v.visitException(this);
    }
    void writeOn(ObjectWriter w) {
	w.write("Exception ");
	w.write(exception);
    }
}

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* visible objects */


class ObjectRegionDB {
  void addObjectRegion(Rectangle rect, TycObject obj) {
    regions.addElement(rect);
    objs.addElement(obj);
  }
  TycObject find(int x, int y) {
    for(int i=regions.size()-1; i>=0; --i) {
      Rectangle r = (Rectangle) regions.elementAt(i);
      if (r.contains(x,y))
	 return (TycObject) objs.elementAt(i);
    }
    return null;
  }
  Vector regions = new Vector();
  Vector objs = new Vector();
}

class IObjectWriter implements ObjectWriter {

  IObjectWriter(int x, int y, Graphics g) {
    this.x = x;
    this.y = y;
    this.g = g;
    fm = g.getFontMetrics();
    int ascent = fm.getAscent();
    y1 = y - ascent;
    height = ascent + fm.getDescent();
  }

  void moveTo(int x, int y) {
    this.x = x;
    this.y = y;
    y1 = y - fm.getAscent();
  }

  public void write(char c) {
    char[] data = { c };
    g.drawChars(data, 0, 1, x, y);
    x += fm.charWidth(c);
  }

  public void write(String s) {
    g.drawString(s, x, y);
    x += fm.stringWidth(s);
  }

  public void write(TycObject o) {
    Color c = g.getColor();
    g.setColor(Color.blue);
    String s = (o == null) ? "nil" : o.name;
    write(s);
    g.setColor(c);
  }

  int x, y;
  Graphics g;
  FontMetrics fm;
  int y1, height;
}

class ObjectRegionWriter implements ObjectWriter {

  ObjectRegionWriter(ObjectRegionDB regionDB, int x, int y, FontMetrics fm) {
    this.regionDB = regionDB;
    this.x = x;
    this.y = y;
    this.fm = fm;
    int ascent = fm.getAscent();
    y1 = y - ascent;
    height = ascent + fm.getDescent();
  }

  void moveTo(int x, int y) {
    this.x = x;
    this.y = y;
    y1 = y - fm.getAscent();
  }

  public void write(char c) {
    x += fm.charWidth(c);
  }

  public void write(String s) {
    x += fm.stringWidth(s);
  }

  public void write(TycObject o) {
    String s = (o == null) ? "nil" : o.name;
    int width = fm.stringWidth(s);
    regionDB.addObjectRegion(new Rectangle(x, y1, width, height), o);
    x += width;
  }

  ObjectRegionDB regionDB;
  int x, y;
  FontMetrics fm;
  int y1, height;
}

class Activation {
    Activation(ThreadInfo thread, TycObject component, TycEvent start) {
	this.thread = thread;
	this.component = component;
	this.start = start;
	this.end = null;
    }
    int startSeqNo() {
	if (start == null)
	    return 0;
	else
	    return start.seqNo;
    }
    int endSeqNo() {
	if (end == null)
	    return Integer.MAX_VALUE;
	else
	    return end.seqNo;
    }
    ThreadInfo thread;
    TycObject component;
    TycEvent start, end;
    int depth;
    Rectangle rect;
}

class ReverseVectorEnumeration implements Enumeration {
    Vector v;
    int i;
    ReverseVectorEnumeration(Vector v) {
	this.v = v;
	i = v.size();
    }
    public boolean hasMoreElements() {
	return (i > 0);
    }
    public Object nextElement() {
	--i;
	return v.elementAt(i);
    }
}

class ThreadInfo  implements TycEventVisitor {
TycObject threadObject;
ThreadInfo(TycObject threadObject) {
  this.threadObject = threadObject;
}
TycEvent lastEvent() {
  if(events.isEmpty()) {
    return null;
  } else {
    return (TycEvent) events.lastElement();
  }
}
void addEvent(TycEvent e) {
  if(e.thread != threadObject)
    throw new Error("Internal error");
  TycEvent lastEvent = lastEvent();
  if(lastEvent != null) {
    if (lastEvent.receivingComponent != e.sendingComponent) {
      System.out.println("Event "+e.seqNo+": previous receiver = "
				+lastEvent.receivingComponent
				+", current sender = "
				+e.sendingComponent
				+"; previous="+lastEvent
				+"; current="+e);
    }
  }
  events.addElement(e);
}
void markAndDeleteDummies() {
    // mark & delete dummies
    int i = 0;
    while(i < events.size()-1) {
	int sendIdx = i;
	TycEvent sendEvt = (TycEvent) events.elementAt(i++);
	TycEvent evt = sendEvt;
	while(evt.receivingComponent == null && i < events.size()-1) {
	    evt = (TycEvent) events.elementAt(i++);
	}
	TycEvent receiveEvt = evt;
	int receiveIdx = i-1;
	if(sendEvt.sendingComponent == receiveEvt.receivingComponent) {
	    for(int j=receiveIdx; j>=sendIdx; --j) {
		evt = (TycEvent) events.elementAt(j);
		evt.seqNo = -1;
		events.removeElementAt(j);  // SLOOOOW
	    }
	    i = sendIdx;
	}
    }
}
void finishBuilding() {
    // construct activations
    for(Enumeration e = getEvents(); e.hasMoreElements(); ) {
	currentEvent = (TycEvent) e.nextElement();
	currentEvent.accept(this);
    }
    currentEvent = null;
    while(!stack.isEmpty()) {
	popFrame();
    }
}
Enumeration getActivations() {
    return new ReverseVectorEnumeration(activations);
}
Enumeration getEvents() {
    return events.elements();
}
private Vector events = new Vector();
// during construction (simulation)
public void visitSend(TycSendEvent evt) {
    if(evt.receivingComponent != evt.sendingComponent) {
	if(evt.sendingComponent != null) {
	    expectTopFrame(evt.sendingComponent);
	    evt.sendingActivation = topFrame();
	    if(evt.sendingComponent != evt.replyToComponent) {
		popFrame();
		if(evt.replyToComponent != null)
		    popFrames(evt.sendingComponent);
	    }
	    if(evt.replyToComponent != null)
		expectTopFrame(evt.replyToComponent);
	}
	if(evt.receivingComponent != null) {
	    pushFrame(evt.receivingComponent);
	    evt.receivingActivation = topFrame();
	}
    } else {
	System.out.println("Doch (1)");
    }
}
public void visitReturn(TycReturnEvent evt) {
    visitReturnoid(evt);
}
public void visitException(TycExceptionEvent evt) {
    visitReturnoid(evt);
}
void visitReturnoid(TycEvent evt) {
    if(evt.receivingComponent != evt.sendingComponent) {
	if(evt.sendingComponent != null) {
	    expectTopFrame(evt.sendingComponent);
	    evt.sendingActivation = topFrame();
	    popFrame();
	    if(evt.receivingComponent != null)
		popFrames(evt.sendingComponent);
	}
	if(evt.receivingComponent != null) {
	    expectTopFrame(evt.receivingComponent);
	    evt.receivingActivation = topFrame();
	}
    } else {
	System.out.println("Doch (2)");
    }
}
void pushFrame(TycObject component) {
    stack.addElement(new Activation(this, component, currentEvent));
}
void popFrames(TycObject leftComponent) {
    int n = 0;
    while(!stack.isEmpty() && topFrame().component == leftComponent) {
	++n;
	popFrame();
    }
    if(n>0)
	System.out.println(""+n+"frames popped");
}
void popFrame() {
  Activation a = (Activation) stack.lastElement();
  a.end = currentEvent;
  activations.addElement(a);
  stack.removeElementAt(stack.size()-1);
}
Activation topFrame() {
  return (Activation) stack.lastElement();
}
void expectTopFrame(TycObject component) {
    if(stack.isEmpty()) {
	Activation a = new Activation(this, component, null);
	stack.addElement(a);
    } else if(((Activation) stack.lastElement()).component != component) {
	System.out.println("unexpected stack contents: "+((Activation) stack.lastElement()).component+", expected "+component);
    }
}
private TycEvent currentEvent;
private Vector stack = new Vector();
private Vector activations = new Vector();
}

class Lifeline {
TycObject component;
Lifeline(TycObject component)
{
  this.component = component;
}
void assignActivationDepths() {
    // for each activation...
    for(Enumeration e1 = activations.elements(); e1.hasMoreElements(); ) {
	Activation a1 = (Activation) e1.nextElement();
	int startSeqNo1 = a1.startSeqNo();
	int endSeqNo1 = a1.endSeqNo();
	// determine all depths that have been assigned to overlapping element
	BitSet usedDepths = new BitSet();
	a1.depth = -1;  // stop marker
	for(Enumeration e2 = activations.elements(); e2.hasMoreElements(); ) {
	    Activation a2 = (Activation) e2.nextElement();
	    int startSeqNo2 = a2.startSeqNo();
	    int endSeqNo2 = a2.endSeqNo();
	    if(startSeqNo2 > endSeqNo1 || a2.depth == -1)
		break;  // no more overlaps, or no more assigned depths
	    if(Math.max(startSeqNo1, startSeqNo2)
	       <= Math.min(endSeqNo1, endSeqNo2)) {
		// overlap
		usedDepths.set(a2.depth);
	    }
	}
	// assign the lowest free depth to a1
	a1.depth = 0;
	while(usedDepths.get(a1.depth))
	    ++a1.depth;
    }
}
void addActivation(Activation a) {
    if(a.component != component)
	System.out.println("Internal error: bad component in Lifeline::addActivation");
    else {
	// sorted insert (binary search)
	// sorted by starting sequence number
	int startSeqNo = a.startSeqNo();
	int endSeqNo = a.endSeqNo();
	int l = 0;
	int r = activations.size()-1;
	while (l<=r) {
	    int m = (l+r)/2;
	    Activation other = (Activation) activations.elementAt(m);
	    if(startSeqNo < other.startSeqNo()
	       || startSeqNo == other.startSeqNo()
	          && endSeqNo > other.endSeqNo())
		r = m-1;
	    else
		l = m+1;
	}
	activations.insertElementAt(a, l);
    }
}

int x;
int middleX;
int width;
Vector activations = new Vector();
}

class SeqDiagramCanvas extends Canvas {

  SeqDiagram applet;
  SeqDiagramCanvas(SeqDiagram applet)
  {
    this.applet = applet;
  }

  ObjectRegionDB regionDB = new ObjectRegionDB();
  
  public void showStatus(String msg) {
      applet.getAppletContext().showStatus(msg);
  }

  public void showDocument(URL u) {
      applet.getAppletContext().showDocument(u);
  }

  public void showDocument(URL u, String where) {
      applet.getAppletContext().showDocument(u, where);
  }

/*------ painting -------*/

  static final int gapBetweenLifelines = 20;
  static final int selfDelegationDx = 20;
  static final int activationWidth = 10;
  static final int recursiveActivationDx = 4;
  boolean keepDummies = false;
  Font labelFont;
  FontMetrics labelFontMetrics;
  int headerHeight;
  int timeStepHeight;
  int eps;
  int labelX;
  int fontAscent;
  int max_width, max_height;
  FontMetrics fm;

  int translate_x = 0, translate_y = 0;

  void measure() {
      fm = getFontMetrics(getFont());
      if(fm==null) return;
      showStatus("Computing activation screen positions");
      regionDB = new ObjectRegionDB();
      eps = fm.charWidth('M');
      headerHeight = fm.getHeight();
      fontAscent = fm.getAscent();
      timeStepHeight = headerHeight;  // ?
      int x = 0;
      for(Enumeration e = lifelinesOrdered.elements(); e.hasMoreElements(); ) {
	Lifeline l = (Lifeline) e.nextElement();
	int width = eps + fm.stringWidth(l.component.name) + eps;
	l.x = x;
	l.width = width;
	l.middleX = x + width/2;
	regionDB.addObjectRegion(new Rectangle(x,0,width,headerHeight),
				 l.component);
	for(Enumeration ea = l.activations.elements(); ea.hasMoreElements();) {
	  Activation a = (Activation) ea.nextElement();
	  int startY =
	      a.start == null
	      ? headerHeight
	      : timeStepY(a.start.seqNo)+timeStepHeight * 3 / 4;
	  int endY =
	      a.end == null
	      ? timeStepY(events.size())
	      : timeStepY(a.end.seqNo)+timeStepHeight / 4;
	  int rx = l.middleX-activationWidth/2 + a.depth * recursiveActivationDx;
	  a.rect = new Rectangle(rx, startY, activationWidth, endY-startY);
	}
	x += width + gapBetweenLifelines;
      }
      labelX = x;
      showStatus("Computing link regions");
      max_width = labelX;
      ObjectRegionWriter orw = new ObjectRegionWriter(regionDB, 0, 0, fm);
      for(Enumeration e = events.elements(); e.hasMoreElements(); ) {
	TycEvent event = (TycEvent) e.nextElement();
	orw.moveTo(x, timeStepY(event.seqNo)+fontAscent);
	event.writeOn(orw);
	max_width = Math.max(max_width, orw.x);
      }
      max_height = timeStepY(events.size())+1;
  }

  public void setLabelFont(Font f) {
      labelFont = f;
  }
  
  public void setFont(Font f) {
      super.setFont(f);
      measure();
      repaint();
  }

  public void setForeground(Color c) {
      super.setForeground(c);
      repaint();
  }

  public void addNotify() {
      super.addNotify();
      measure();
  }

  public Dimension preferredSize() {
      return new Dimension(max_width, max_height);
  }

  public Dimension minimumSize() {
      return new Dimension(max_width, timeStepY(5));
  }

  public void paint(Graphics g) {
    g.translate(-translate_x, -translate_y);
    Rectangle clip = g.getClipRect();

    for(Enumeration e = lifelinesOrdered.elements(); e.hasMoreElements(); ) {
      Lifeline l = (Lifeline) e.nextElement();
      g.drawRect(l.x,0,l.width,headerHeight);
      g.drawString(l.component.name, l.x+eps, fontAscent);
      g.drawLine(l.middleX, headerHeight,
		 l.middleX, timeStepY(events.size()));
      for(Enumeration ea = l.activations.elements(); ea.hasMoreElements();) {
        Activation a = (Activation) ea.nextElement();
	Rectangle r = a.rect;
	g.setColor(Color.white);
	g.fillRect(r.x,r.y,r.width,r.height);
	g.setColor(getForeground());
	g.drawRect(r.x,r.y,r.width,r.height);
      }
    }
    if(clip.x + clip.width >= labelX) {
	IObjectWriter ow = new IObjectWriter(0, 0, g);
	int lastEventIdx =
	    Math.min(events.size()-1,
		     (clip.y+clip.height-headerHeight) / timeStepHeight);
	for(int i = Math.max(0, (clip.y-headerHeight) / timeStepHeight);
	    i <= lastEventIdx;
	    ++i) {
	  TycEvent event = (TycEvent) events.elementAt(i);
	  if(event.seqNo != i)
	      System.out.println("Bad seqno: "+event.seqNo+", expected "+i);
	  ow.moveTo(labelX, timeStepY(i)+fontAscent);
	  event.writeOn(ow);
	}
    }

    g.setFont(labelFont);
    labelFontMetrics = getFontMetrics(labelFont);
    int labelFontHeight = labelFontMetrics.getHeight();
    for(Enumeration t = threads.elements(); t.hasMoreElements(); ) {
      ThreadInfo thread = (ThreadInfo) t.nextElement();
      Enumeration e = thread.getEvents();
      while(e.hasMoreElements()) {
        TycEvent sendEvt = (TycEvent) e.nextElement();
        Lifeline sendLifeline = getLifeline(sendEvt.sendingComponent);
	TycEvent evt = sendEvt;
	while(evt.receivingComponent == null && e.hasMoreElements()) {
	  evt = (TycEvent) e.nextElement();
	}
	TycEvent receiveEvt = evt;

        Lifeline receiveLifeline = getLifeline(receiveEvt.receivingComponent);
	int sendY = timeStepY(sendEvt.seqNo) + timeStepHeight / 4;
	int receiveY = timeStepY(receiveEvt.seqNo) + timeStepHeight * 3 / 4;
	if(sendY-labelFontHeight > clip.y+clip.height || receiveY < clip.y)
	    continue;
	if(sendLifeline == null || receiveLifeline == null) {
	  if(receiveLifeline != null) {
	    // receive without a sender
	    Rectangle r = receiveEvt.receivingActivation.rect;
	    int sideX = r.x - receiveLifeline.width/3;
	    drawArrow(g, receiveEvt,
		      sideX, receiveY - timeStepHeight / 4, r.x, receiveY);
	  }
	  if(sendLifeline != null) {
	    // send without a receiver
	    Rectangle r = sendEvt.sendingActivation.rect;
	    int sideX = r.x - sendLifeline.width / 3;
	    drawArrow(g, sendEvt,
		      r.x, sendY, sideX, sendY + timeStepHeight / 4);
	  }
	} else if(sendLifeline == receiveLifeline) {
	  int sendMiddleX = sendEvt.sendingActivation.rect.x + activationWidth;
	  int receiveMiddleX = receiveEvt.receivingActivation.rect.x + activationWidth;
	  int sideX = Math.max(sendMiddleX,receiveMiddleX) + selfDelegationDx;
	  if(sendEvt == receiveEvt) {
	    g.drawLine(sendMiddleX, sendY, sideX, sendY);
	    g.drawLine(sideX, sendY, sideX, receiveY);
	    drawArrow(g, sendEvt, sideX, receiveY, receiveMiddleX, receiveY);
	  } else {
	    drawArrow(g, sendEvt,
		      sendMiddleX, sendY, sideX, sendY + timeStepHeight / 4);
	    g.setColor(Color.gray);
	    g.drawLine(sideX, sendY + timeStepHeight / 4,
		       sideX, receiveY - timeStepHeight / 4);
	    g.setColor(getForeground());
	    drawArrow(g, receiveEvt,
			sideX, receiveY - timeStepHeight / 4, receiveMiddleX, receiveY);
	  }
	} else {
	  int x1 = sendEvt.sendingActivation.rect.x;
	  int y1 = sendY;
	  int x2 = receiveEvt.receivingActivation.rect.x;
	  int y2 = receiveY;
	  if(x1<x2) {
	    x1 += activationWidth;
	  } else {
	    x2 += activationWidth;
	  }
	  if(sendEvt == receiveEvt) {
	    drawArrow(g, sendEvt, x1,y1,x2,y2);
	  } else {
	    int dx = (x2-x1)/3;
	    int dy = (y2-y1)/3;
	    drawArrow(g, sendEvt, x1,y1, x1+dx,y1+dy);
	    g.setColor(Color.gray);
	    g.drawLine(x1+dx,y1+dy, x2-dx,y2-dy);
	    g.setColor(getForeground());
	    drawArrow(g, receiveEvt, x2-dx,y2-dy, x2,y2);
	  }
	}
      }
    }
  }

  static final int arrayHeadWidth = 5;
  static final int arrayHeadLength = 10;
  
  void drawArrow(Graphics g, TycEvent evt, int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    double norm = Math.sqrt(dx*dx + dy*dy);
    double vx = dx / norm;
    double vy = dy / norm;
    double x3 = x2 - arrayHeadLength*vx;
    double y3 = y2 - arrayHeadLength*vy;
    int x[] = { (int)(x3 - arrayHeadWidth*vy), x2, (int)(x3 + arrayHeadWidth*vy) };
    int y[] = { (int)(y3 + arrayHeadWidth*vx), y2, (int)(y3 - arrayHeadWidth*vx) };
    if(evt instanceof TycSendEvent) {
      String label = ((TycSendEvent)evt).selector.symbol;
      int labelCenterX = labelFontMetrics.stringWidth(label) / 2;
      int labelCenterY = labelFontMetrics.getHeight() / 2;
      double cx =  labelCenterX*vx - labelCenterY*vy;
      //double cy = labelCenterX*vy + labelCenterY*vx;
      int xm = (x2+x1)/2;
      int ym = (y2+y1)/2;
      int bx = xm - labelCenterX;
      int by;
      if(x2>x1) { 
        by = ym - (int)(cx * vy);
      } else {
	by = ym + (int)(cx * vy);
      }
      if(by < y1)
	  by = y1;
      // g.drawRect(bx,by-2*labelCenterY,2*labelCenterX,2*labelCenterY);
      g.setColor(Color.magenta);
      g.drawString(label, bx, by-fm.getDescent());
      g.setColor(getForeground());
      g.drawLine(x1,y1,(int)x3,(int)y3);
      g.fillPolygon(x,y,3);
    } else if(evt instanceof TycReturnEvent) {
      g.drawLine(x1,y1,x2,y2);
      g.drawLine(x[0],y[0],x[1],y[1]);  // netscape lacks drawPolyline
      g.drawLine(x[1],y[1],x[2],y[2]);
    } else if(evt instanceof TycExceptionEvent) {
      g.setColor(Color.red);
      g.drawLine(x1,y1,x2,y2);
      g.drawLine(x[0],y[0],x[1],y[1]);  // netscape lacks drawPolyline
      g.drawLine(x[1],y[1],x[2],y[2]);
      g.setColor(getForeground());
    }
  }

    
  int timeStepY(int timeStep)
  {
    return headerHeight + timeStep * timeStepHeight;
  }

/*------ hot links ----------------------*/

  public boolean mouseMove(Event evt, int x, int y) {
    TycObject obj = regionDB.find(x+translate_x,y+translate_y);
    if (obj != null) {
      showStatus(obj.name+": "+obj.Oid);
    } else {
      showStatus("");
    }
    return false;
  }

  public boolean mouseDown(Event evt, int x, int y) {
    TycObject obj = regionDB.find(x+translate_x,y+translate_y);
    if (obj != null) {
      try {
	URL base = applet.getDocumentBase();
	URL url =
	  new URL(base.getProtocol(), base.getHost(), base.getPort(),
	    "/admin/object.stml?oid="+obj.Oid
		  +(applet.session != null ? "&session="+applet.session : "")
		  +(applet.step != null ? "&step="+applet.step : "")
		  +"&fork=");
	showDocument(url, "object");
      } catch (Throwable e) {
	showStatus(e.getMessage());
      }
      return true;
    }
    return false;
  }


/*------ building data structures -------*/

  void addTycEvent(TycEvent event)
  {
    event.seqNo = events.size();
    events.addElement(event);
    ThreadInfo threadInfo = getThreadInfo(event.thread);
    Lifeline sendingLifeline = getLifeline(event.sendingComponent);
    Lifeline receivingLifeline = getLifeline(event.receivingComponent);
    threadInfo.addEvent(event);
  }

  void finishBuilding() {
    showStatus("Computing activations");
    for(Enumeration e = threads.elements(); e.hasMoreElements(); ) {
      ThreadInfo thread = (ThreadInfo) e.nextElement();
      if(!keepDummies)
	  thread.markAndDeleteDummies();
      thread.finishBuilding();
      for(Enumeration ea = thread.getActivations(); ea.hasMoreElements(); ) {
	  Activation activation = (Activation) ea.nextElement();
	  getLifeline(activation.component).addActivation(activation);
      }
    }
    showStatus("Assigning activation depths");
    for(Enumeration e = lifelines.elements(); e.hasMoreElements(); ) {
      Lifeline lifeline = (Lifeline) e.nextElement();
      lifeline.assignActivationDepths();
    }
    if(!keepDummies) {
	showStatus("Removing untraced data paths");
	for(int i = 0; i < events.size(); ) {
	    TycEvent event = (TycEvent) events.elementAt(i);
	    if(event.seqNo == -1)
		events.removeElementAt(i);
	    else
		event.seqNo = i++;
	}
    }
    showStatus("");
  }

  ThreadInfo getThreadInfo(TycObject thread)
  {
    ThreadInfo result = (ThreadInfo) threads.get(thread);
    if (result == null) {
      result = new ThreadInfo(thread);
      threads.put(thread, result);
    }
    return result;
  }

  Lifeline getLifeline(TycObject component)
  {
    if(component == null) {
      return null;
    } else {
      Lifeline result = (Lifeline) lifelines.get(component);
      if (result == null) {
	result = new Lifeline(component);
	lifelinesOrdered.addElement(result);
	lifelines.put(component, result);
      }
      return result;
    }
  }

  Vector events = new Vector();
  Vector lifelinesOrdered = new Vector();
  Hashtable lifelines = new Hashtable();
  Hashtable threads = new Hashtable();
}

public class SeqDiagram extends Applet {

    SeqDiagramCanvas canvas;
    Scrollbar vbar;
    Scrollbar hbar;
    Vector objects;
    String session;
    String step;

    /**
     * Initializes the applet.  You never need to call this directly; it is
     * called automatically by the system once the applet is created.
     */
    public void init() {
      session = getParameter("session");
      step = getParameter("step");
      canvas = new SeqDiagramCanvas(this);
      parseObjects();
      parseEvents();
      canvas.finishBuilding();
      String fontName = getStringParameter("fontname", "Helvetica");
      int fontSize = getIntParameter("fontsize", 14);
      canvas.setFont(new Font(fontName,Font.PLAIN,fontSize));
      canvas.setLabelFont(new Font(fontName,Font.PLAIN,fontSize-2));
      canvas.keepDummies = getBoolParameter("keepdummies", false);
      vbar = new Scrollbar(Scrollbar.VERTICAL);
      hbar = new Scrollbar(Scrollbar.HORIZONTAL);
      setLayout(new BorderLayout());
      add("Center", canvas);
      add("East", vbar);
      add("South", hbar);
      updateScrollbar();
    }

    public boolean handleEvent(Event e) {
	// handle AWT event, that is
	if(e.target == vbar) {
	    switch(e.id) {
	    case Event.SCROLL_LINE_UP:
	    case Event.SCROLL_LINE_DOWN:
	    case Event.SCROLL_PAGE_UP:
	    case Event.SCROLL_PAGE_DOWN:
	    case Event.SCROLL_ABSOLUTE:
		canvas.translate_y = ((Integer)e.arg).intValue();
		canvas.repaint();
		break;
	    }
	    return true;
	}
	if(e.target == hbar) {
	    switch(e.id) {
	    case Event.SCROLL_LINE_UP:
	    case Event.SCROLL_LINE_DOWN:
	    case Event.SCROLL_PAGE_UP:
	    case Event.SCROLL_PAGE_DOWN:
	    case Event.SCROLL_ABSOLUTE:
		canvas.translate_x = ((Integer)e.arg).intValue();
		canvas.repaint();
		break;
	    }
	    return true;
	}
	return super.handleEvent(e);
    }

    public synchronized void reshape(int x, int y, int width, int height) {
	super.reshape(x,y,width,height);
	if(canvas != null && vbar != null) {
	  updateScrollbar();
	  update(getGraphics());
	}
    }

    static final boolean forJdk102 = false;
    void scrollbarSetValues(Scrollbar bar, int value, int visible, int min, int max) {
      if(forJdk102)
        max = Math.max(0, max-visible);
      bar.setValues(value, visible, min, max);
    }

    void updateScrollbar() {
	int canvas_height = size().height;  // full height of applet
	int maxHeight = Math.max(0, canvas.max_height-canvas_height);
	canvas.translate_y = Math.min(maxHeight, canvas.translate_y);
	scrollbarSetValues(vbar,canvas.translate_y,
		      canvas_height,
		      0, canvas.max_height);
	vbar.setPageIncrement(canvas_height / 2);
	vbar.setLineIncrement(canvas.timeStepHeight);

	int canvas_width = size().width;  // full width of applet
	int maxWidth = Math.max(0, canvas.max_width-canvas_width);
	canvas.translate_x = Math.min(maxWidth, canvas.translate_x);
	scrollbarSetValues(hbar,canvas.translate_x,
		      canvas_width,
		      0, canvas.max_width);
	hbar.setPageIncrement(canvas_width / 2);
	hbar.setLineIncrement(10);
    }
    
    void parseObjects()
    {
	showStatus("Parsing objects");
	objects = new Vector();
	String objectInfo;
	objects.addElement(null);
	for(int n=1; (objectInfo = getParameter("object"+n)) != null; ++n) {
	    objects.addElement(new TycObject(objectInfo));
	}
    }

    void parseEvents()
    {
      String eventInfo;
      showStatus("Parsing events");
      for(int n=1; (eventInfo = getParameter("event"+n)) != null; ++n) {
	if(n%20 == 0)
          showStatus("Parsing events ("+n+")");
        StringTokenizer stoke = new StringTokenizer(eventInfo);
	TycEvent event = null;
	try {
	  int nTokens = stoke.countTokens();
	  if(nTokens >= 6) {
	    String subclass = stoke.nextToken();
	    TycObject thread = strIdToObject(stoke.nextToken());
	    TycObject sender = strIdToObject(stoke.nextToken());
	    TycObject sendingComponent = strIdToObject(stoke.nextToken());
	    TycObject receiver = strIdToObject(stoke.nextToken());
	    TycObject receivingComponent = strIdToObject(stoke.nextToken());
	    if(subclass.equals("Send")) {
	      if(nTokens >= 9) {
		TycSendEvent sendEvent = new TycSendEvent();
		sendEvent.replyTo = strIdToObject(stoke.nextToken());
		sendEvent.replyToComponent = strIdToObject(stoke.nextToken());
		sendEvent.selector = new TycSelector(stoke.nextToken());
		int arity = sendEvent.selector.arity;
		if(nTokens == 9+arity) {
		  sendEvent.args = new TycObject[arity];
		  for(int i=0; i<arity; ++i) {
		    sendEvent.args[i] = strIdToObject(stoke.nextToken());
		  }
		  event = sendEvent;
		}
	      }
	    } else if (subclass.equals("Return")) {
	      TycReturnEvent returnEvent = new TycReturnEvent();
	      returnEvent.result = strIdToObject(stoke.nextToken());
	      returnEvent.isComponent = stoke.hasMoreElements();
	      event = returnEvent;
	    } else if (subclass.equals("Exception")) {
	      TycExceptionEvent exceptionEvent = new TycExceptionEvent();
	      exceptionEvent.exception = strIdToObject(stoke.nextToken());
	      event = exceptionEvent;
	    }
	    if (event != null) {
	      event.thread = thread;
	      event.sender = sender;
	      event.sendingComponent = sendingComponent;
	      event.receiver = receiver;
	      event.receivingComponent = receivingComponent;
	    }
	  }
	} catch (Exception e) {
	  System.out.println("Exception while parsing event "+n+": "+e);
	}
	if (event != null) {
	  canvas.addTycEvent(event);
	} else {
	  System.out.println("Could not parse event "+n+": `"+eventInfo+"'");
	}
      }
    }

    TycObject strIdToObject(String str)
    {
      return (TycObject) objects.elementAt(Integer.parseInt(str));
    }
    
    /**
     * Called to start the applet.  You never need to call this directly; it
     * is called when the applet's document is visited.
     */
    public void start() {
	//
    }

    /**
     * Called to stop the applet.  This is called when the applet's document is
     * no longer on the screen.  It is guaranteed to be called before destroy()
     * is called.  You never need to call this method directly
     */
    public void stop() {
	//
    }

    /**
     * Cleans up whatever resources are being held.  If the applet is active
     * it is stopped.
     */
    public void destroy() {
    }

    String getStringParameter(String prop, String defaultValue) {
      String propValue;
      if ((propValue = getParameter(prop)) != null)
	return propValue;
      else
	return defaultValue;
    }

    int getIntParameter(String prop, int defval) {
      int result = defval;
      String propVal;
      if ((propVal = getParameter(prop)) != null) {
	try {
	  result = Integer.parseInt(propVal);
	} catch(NumberFormatException e) {
	    System.out.println("Strange integer parameter for "+prop+": "+propVal);
	}
      }
      return result;
    }

    boolean getBoolParameter(String prop, boolean defval) {
      boolean result = defval;
      String propVal;
      if ((propVal = getParameter(prop)) != null) {
	  if(propVal.equals("true"))
	      result = true;
	  else if(propVal.equals("false"))
	      result = false;
	  else
	      System.out.println("Strange boolean parameter for "+prop+": "+propVal);
      }
      return result;
    }
}

