import java.util.*;
import java.awt.*;
import java.applet.*;
import java.net.*;

class LayoutInfo {
  int horizontalDistance;
  int verticalDistance;
  int margin;
  int nodeHeight;
  Font abstractFont;
  Font concreteFont;
  FontMetrics abstractFm;
  FontMetrics concreteFm;
  
  LayoutInfo(int horizontalDistance,
	     int verticalDistance,
	     Font abstractFont,
	     Font concreteFont,
	     FontMetrics abstractFm,
	     FontMetrics concreteFm,
	     int margin,
	     int nodeHeight) {
    this.horizontalDistance = horizontalDistance;
    this.verticalDistance = verticalDistance;
    this.abstractFont = abstractFont;
    this.concreteFont = concreteFont;
    this.abstractFm = abstractFm;
    this.concreteFm = concreteFm;
    this.margin = margin;
    this.nodeHeight = nodeHeight;
    }
}

class Node {
  int x = 0;
  int y = 0;
  int h;
  int w;

  String label;
  
  Vector parents;
  Vector children;
  int rank;

  boolean isAbstract;
  boolean isVisible;

  Node principal;
  

  int subTreeWidth;
  int subTreeHeight;
  
  void init(LayoutInfo layoutInfo) {
    FontMetrics fm =
      isAbstract ? layoutInfo.abstractFm : layoutInfo.concreteFm;
    w = fm.stringWidth(label) + 2 * layoutInfo.margin;
    h = layoutInfo.nodeHeight;
    subTreeHeight = subTreeWidth = 0;
    x = y = 0;
    rank = Integer.MIN_VALUE;
  }
   

  void layoutX(LayoutInfo layoutInfo) {
    for (Enumeration e = parents.elements(); e.hasMoreElements(); ) {
      Node p = (Node)e.nextElement();
      int newx = p.x + p.w + layoutInfo.horizontalDistance;
      x = Math.max(x, newx);
      if (rank < p.rank+1) {
        rank = p.rank+1;
	/*principal = p;*/
      if (principal == null)
        principal = p;
      }      
    }
  }
  
  void layoutY1(LayoutInfo layoutInfo) {
    subTreeWidth = Math.max(subTreeWidth, w);
    subTreeHeight = Math.max(h, subTreeHeight);
    if (principal != null) {
      principal.subTreeHeight += subTreeHeight;
      principal.subTreeWidth =
        Math.max(principal.subTreeWidth, (x-principal.x)+subTreeWidth);
    }
    }
    
  void layoutY2(LayoutInfo layoutInfo) {
    int newy = y - subTreeHeight/2;
    for (Enumeration e = children.elements(); e.hasMoreElements(); ) {
      Node c = (Node)e.nextElement();
      if (c.principal == this) {
        c.y = newy + c.subTreeHeight/2;
	newy += c.subTreeHeight;
      }
    }
  }
    
  public void paint(Graphics g, LayoutInfo layoutInfo) {
    if (x == 0 && y == 0) 
      {
      System.out.print("Root Node: ");
      System.out.println(label);
      }
    Font font = isAbstract ? layoutInfo.abstractFont : layoutInfo.concreteFont;
    FontMetrics fm = isAbstract ? layoutInfo.abstractFm : layoutInfo.concreteFm;
    g.setFont(font);
    g.drawString(label,
      x+layoutInfo.margin,
      y - layoutInfo.nodeHeight/2 + fm.getAscent());
    for (Enumeration e = parents.elements(); e.hasMoreElements(); ) {
      Node n = (Node)e.nextElement();
      g.drawLine(x,y,n.x+n.w,n.y);
    }
  }
  
  public String pick(int mx, int my) {
    if (mx >= x && mx < x+w && my >= y-h/2 && my < y+h/2)
      return label;
    for (Enumeration e = children.elements(); e.hasMoreElements(); ) {
      Node n = (Node)e.nextElement();
      String name = n.pick(mx,my);
      if (name.length() > 0)
        return name;
    }
    return "";
  }
}

class GraphPanel extends Panel implements Runnable {

    GraphPanel(DAG graph) {
      this.graph = graph;
    }

    String urlPrefix, urlPostfix;
    DAG graph;
    Vector nodes = new Vector();

    boolean layouted = false; 
    Thread layouter;
    LayoutInfo layoutInfo = null;
    int translateX = 0;
    int translateY = 0;

    Node root() {
      return (Node) nodes.elementAt(0);
    }
    
    public void showStatus(String msg) {
	graph.getAppletContext().showStatus(msg);
    }

    public void showDocument(URL u) {
	graph.getAppletContext().showDocument(u, "Interface View");
    }

    Node findNode(String label) {
      String all = "";
      for (int i = 0; i < nodes.size(); i++) {
        Node n = (Node) nodes.elementAt(i);
        all = all+n.label;
        if (n.label.equals(label)) {
      	  return n;
        }
      }
      String msg = "Node not found:  '" + label + "'";
      throw new RuntimeException(msg);
    }
    
    Node addNode(String label) {
	Node n = new Node();
	n.label = label;
	n.parents = new Vector();
	n.children = new Vector();
	nodes.addElement(n);
	return n;
    }
    
    public void paint(Graphics g) {
      if (!layouted && layouter == null) {
        int fontSize = 10;
        Font abstractFont = new Font("Helvetica",Font.PLAIN,fontSize);
        Font concreteFont = new Font("Helvetica",Font.BOLD,fontSize);
       
	FontMetrics abstractFm = g.getFontMetrics(abstractFont);
        FontMetrics concreteFm = g.getFontMetrics(concreteFont);
	
	int eps = concreteFm.charWidth('m')/4;
        layoutInfo =
	  new LayoutInfo(4 * 4 * eps,
			 2 * concreteFm.getHeight(),
			 abstractFont,
			 concreteFont,
			 abstractFm,
			 concreteFm,
			 eps,
			 concreteFm.getHeight());
	layouter = new Thread(this);
	layouter.start();
      } else {
	Dimension d = size();
	g.setColor(Color.black);
	translateX = 0;
	translateY = root().subTreeHeight/2;
	g.translate(translateX,translateY);
	for (Enumeration e = nodes.elements(); e.hasMoreElements(); ) {
	  Node n = (Node)e.nextElement();
	  n.paint(g,layoutInfo);
	}
      }
    }

    public synchronized boolean mouseDown(Event evt, int x, int y) {
      String className = root().pick(x-translateX,y-translateY);
      if (className.length() > 0)
	try {
	  URL base = graph.getDocumentBase();
	  URL url = new URL(base, urlPrefix + className + urlPostfix);
	  System.err.println(url);
	  showDocument(url);
	} catch (Throwable e) {
	  showStatus(e.getMessage());
	}
    return true;	
    }

    public void start() {
    }

    public void stop() {
      if (layouter != null) {
	layouter.stop();
	layouter = null;
      }
    }

    public void run() {
      /* init: */
      for (int i = 0; i < nodes.size(); i++ ) {
	Node n = (Node)nodes.elementAt(i);
	n.init(layoutInfo);
      }
      /* top down: */
      for (int i = 0; i < nodes.size(); i++ ) {
	Node n = (Node)nodes.elementAt(i);
	n.layoutX(layoutInfo);
      }
      /* bottom up: */
      for (int i = nodes.size()-1; i >=0; i-- ) {
	Node n = (Node)nodes.elementAt(i);
	n.layoutY1(layoutInfo);
      }
      /* top down: */
      for (int i = 0; i < nodes.size(); i++ ) {
	Node n = (Node)nodes.elementAt(i);
	n.layoutY2(layoutInfo);
      }

      graph.resize(root().subTreeWidth,root().subTreeHeight);
      layouted = true;
      layouter = null;
      repaint();
    }

}

public class DAG extends Applet {
    GraphPanel panel;

    public void init() {
      setLayout(new BorderLayout());
      
      panel = new GraphPanel(this);

      try {
	String nodeString = getParameter("nodes");

	panel.urlPrefix = getParameter("urlPrefix");
	panel.urlPostfix = getParameter("urlPostfix");

	for (StringTokenizer t = new StringTokenizer(nodeString) ;
	     t.hasMoreTokens() ; ) {
	  String childString = t.nextToken();
	  int i = childString.indexOf('-');
	  if (i > 0) {
	    boolean isAbstract = false;
	    int start = 0;
	    if (childString.charAt(0) == '$')  {
	      isAbstract = true;
	      start = 1;
	      }
	    Node child = panel.addNode(childString.substring(start,i));
	    child.isAbstract = isAbstract;
	    
	    for (StringTokenizer pt = new StringTokenizer(childString.substring(i+1), ",");
		 pt.hasMoreTokens(); ) {
	      String parentString = pt.nextToken();
	      Node paren = panel.findNode(parentString);
	      child.parents.addElement(paren);
	      paren.children.addElement(child);
	      }
	   }
	 }
        add("Center", panel);
      } catch (Exception e) {
        add("West", new Label("Exception: " + e.getMessage()));	
      }
    }

    public void start() {
      panel.start();
    }
    public void stop() {
      panel.stop();
    }
    
}
