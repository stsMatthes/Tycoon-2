//	Author: C Dare-Edwards
//	
//	cdare-edwards@mcsoftware.com.au
//	
//  Copyright Conrad Dare-Edwards   
//
//  The copyright to the computer program (s) herein
//  is the property of Conrad Dare-Edwards. The program (s)
//  may be used and/or copied only with the written 
//  permission of Conrad Dare-Edwards or in accordance 
//  with the terms and conditions stipulated in the 
//  agreement/contract under which the program (s) have
//  been supplied.

import java.applet.*;
import java.awt.*;
import java.net.URL;
import java.net.MalformedURLException;
import java.util.StringTokenizer;
import java.util.Hashtable;
import java.util.Enumeration;

public class TreeApp extends Applet implements Runnable
{
	private String m_rootTitle = "Root Directory";
	private boolean m_expanded = false;
	private String m_baseURLString;
	private URL m_baseURL;

	private TreeControl m_control;

	FolderItem m_folder;	
	private Hashtable m_imageList;

	private Thread loadThread = null;

    // Parameter names.  To change a name of a parameter, you need only make
	// a single change.  Simply modify the value of the parameter string below.
    //--------------------------------------------------------------------------
	private final String PARAM_rootTitle = "rootTitle";
	private final String PARAM_expanded = "expanded";
	private final String PARAM_baseURL = "baseURL";

	private final String PARAM_bgImage = "bgImage";
	private final String PARAM_bgColor = "bgColor";
	private final String PARAM_fgColor = "fgColor";
	private final String PARAM_bgHighlight = "bgHighlight";

	private final String PARAM_textColor = "textColor";
	private final String PARAM_textHighlight = "textHighlight";
	private final String PARAM_font = "font";
	private final String PARAM_offset = "offset";
	private final String PARAM_dotted = "dotted";
	private final String PARAM_showURL = "showURL";
	
	public TreeApp()
	{
	}

	public String getAppletInfo()
	{
		return "Name: TreeApp\r\n" +
		       "Author: Conrad Dare-Edwards\r\n" +
		       "Created with Microsoft Visual J++ Version 1.1\r\n" +
		       "WWW:  http://www.mcsoftware.com.au\r\n" +
		       "E-Mail: cdare-edwards@mcsoftware.com.au";
	}
/*
	public String[][] getParameterInfo()
	{
		String[][] info =
		{
			{ PARAM_rootTitle, "String", "Title for top directory" },
			{ PARAM_expanded, "boolean", "Root directory expanded ?" },
			{ PARAM_baseURL, "String", "URL base for links" },
			{ PARAM_bgImage, "String", "Background image URL" },
			{ PARAM_bgColor, "String", "Window background color" },
			{ PARAM_fgColor, "String", "Color of the link lines" },
			{ PARAM_bgHighlight, "String", "Background color for hightlighted text"},
			{ PARAM_textColor, "String", "Text color" },
			{ PARAM_textHighlight, "String", "Highlighted text color" },
			{ PARAM_font, "String", "Font to be used" },
			{ PARAM_offset, "String", "Directory offset from 0,0" },
			{ PARAM_dotted, "boolean", "Draw dotted lines" },
			{ PARAM_showURL, "boolean", "Show URL on status line" }

		};
		return info;		
	}
*/
	// init applet and load prama's
	public void init()
	{
		m_folder = new FolderItem("");
		m_imageList = new Hashtable();
		
		String param;

		param = getParameter(PARAM_expanded);
		if (param != null)
			m_expanded = Boolean.valueOf(param).booleanValue();

		// draw dotted lines
		boolean dotted = true;
		param = getParameter(PARAM_dotted);
		if (param != null) dotted = Boolean.valueOf(param).booleanValue();

		// set dotted lines 
		FolderItem.m_drawDotted = dotted;

		// setup show URL in ItemAction
		boolean showURL = true;
		param = getParameter(PARAM_showURL);
		if (param != null)
			 showURL = Boolean.valueOf(param).booleanValue();

		ItemAction.m_showURL = showURL;	

		// setup our control
		// use a border layout manager
		this.setLayout(  new BorderLayout(0,0) );

		m_control = new TreeControl();

		// add components to our container
		this. add( "Center", m_control );

		// set default font
		Font font = getParamFont( PARAM_font );
		if( font == null ) font = new Font( "Dialog", 0 ,10);

		// and set it
		if(font != null) m_control.setFont( font );

		// set background image
		param = getParameter( PARAM_bgImage );
		if (param != null)
		{	
			m_control.setBackgroundImage( addImage( param, this ) );
		}

		// setup colors
		Color color;

		// background color
		if( (color = getParamColor( PARAM_bgColor )) != null )
			m_control.m_bgColour = color;

		// highlighted text background
		if( (color = getParamColor( PARAM_bgHighlight )) != null )
			m_control.m_highColour = color;
		
		// detail color 
		if( (color = getParamColor( PARAM_fgColor )) != null )
			m_control.m_detailColour = color;
		
		// text color
		if( (color = getParamColor( PARAM_textColor )) != null )
				m_control.m_textColour = color;
		
		// hightlighted text color
		if( (color = getParamColor( PARAM_textHighlight )) != null )
			m_control.m_highTextColour = color;

		// get and set directory offset
		Point point;
		if( (point = getParamPoint( PARAM_offset )) != null )
			m_control.setOffset( point.x, point.y );			
		

		// load images and draw control
		m_control.initalise( );

		loadThread = new Thread(this);
		loadThread.start();

	}

	private Color getParamColor( String param )
	{
		Color colour = Color.white;	

		param.trim();

		String parameterString;
		if( (parameterString = getParameter( param ) ) != null )
		{
			if( parameterString.compareTo("black")== 0 )		colour = Color.black;
 			else if(parameterString.compareTo("blue")== 0 )		colour = Color.blue;
			else if(parameterString.compareTo("cyan")== 0 )		colour = Color.cyan;
			else if(parameterString.compareTo("darkgray")== 0 ) colour = Color.darkGray;
			else if(parameterString.compareTo("gray")== 0 )		colour = Color.gray;
			else if(parameterString.compareTo("green")== 0 )	colour = Color.green;
			else if(parameterString.compareTo("lightgray")== 0 ) colour = Color.lightGray;
			else if(parameterString.compareTo("magenta")== 0 )	colour = Color.magenta;
			else if(parameterString.compareTo("orange")== 0 )	colour = Color.orange;
			else if(parameterString.compareTo("pink")== 0 )		colour = Color.pink;
			else if(parameterString.compareTo("red")== 0 )		colour = Color.red;
			else if(parameterString.compareTo("white")== 0 )	colour = Color.white;
			else if(parameterString.compareTo("yellow") == 0 )	colour = Color.yellow;
			else if(parameterString.compareTo("rose") == 0 )	colour = new Color(153,0,66);
			else // try as a hex constant
			{
				try {
					
					Integer integer = Integer.valueOf( parameterString, 16 );
					
					if( integer != null ) colour = new Color( integer.intValue() );
				}
				catch( NumberFormatException e )
				{
					return null;
				}
			}
			return colour;
		}
		return null;	
	}

	private Font getParamFont( String param )
	{
		String parameterString;
		param.trim();

		if( (parameterString = getParameter( param ) ) != null )
		{

			String fontName = "Helvetica";
			int style = Font.PLAIN;
			int size = 12;

			StringTokenizer stoke = new StringTokenizer( parameterString, "-", false );

			if( stoke.hasMoreElements() )
				fontName = stoke.nextToken();
			
			if( stoke.hasMoreElements() )
			{
				String styleString = stoke.nextToken();

				styleString = styleString.toLowerCase();
			
				if( styleString.compareTo("plain") == 0 )
					style = Font.PLAIN;

				if( styleString.compareTo("italic") == 0 )
					style = Font.ITALIC;

				if( styleString.compareTo("bold") == 0 )
					style = Font.BOLD;

				if( styleString.compareTo("bolditalic") == 0 )
					style = Font.BOLD + Font.ITALIC;
		
			}// end get style
			
				
			if( stoke.hasMoreElements() )
			{
				try{
					Integer integer = Integer.valueOf(stoke.nextToken());
					size = integer.intValue();
				}
				catch( NumberFormatException e )
				{
					size = 12;
				}
			} // end get size
			

			return new Font( fontName, style, size );
		}
		return null;
	}

	private Point getParamPoint( String param )
	{
		String parameterString;
		Point point = new Point(0,0);

		param.trim();

		if( (parameterString = getParameter( param ) ) != null )
		{
			StringTokenizer stoke = new StringTokenizer( parameterString, ",", false );

			// get x point
			if( stoke.hasMoreElements() )
			{
				try {
					
					Integer integer = Integer.valueOf( stoke.nextToken() );
					
					if( integer != null ) point.x = integer.intValue();
				}
				catch( NumberFormatException e )
				{
					return null;
				}
			
			}		
			// get y point
			if( stoke.hasMoreElements() )
			{
				try {
					
					Integer integer = Integer.valueOf( stoke.nextToken() );
					
					if( integer != null ) point.y = integer.intValue();
				}
				catch( NumberFormatException e )
				{
					return null;
				}
			
			}		
		}
		return point;
	}

	// add an item to the control on the fly
	public void add( String string )
	{
		FolderItem old_folder = m_folder;
		m_folder = m_control.getFolder();	
		
		addItem( string, null,  this, m_baseURL, false );

		// load images
		MediaTracker tracker = new MediaTracker( this );

		// pull all the images our of the hashtable
		// and add them to the tracker
		Enumeration en = m_imageList.elements();
		while( en.hasMoreElements() )
		{
			tracker.addImage( (Image)en.nextElement(), 0 );			
		}

		// load all the images
		try
		{
			tracker.waitForAll();
		}
		catch( InterruptedException e )
		{
			// do nothing as compontents will handle image problems
		}

		m_control.setFolder( m_folder );	
		m_folder = old_folder;
	}

	// delete item from list
	public void delete( String itemTitle )
	{
		itemTitle = getArgument( itemTitle, 0 );
		itemTitle.trim();

		// break string down into elements
		StringTokenizer stoke = new StringTokenizer(itemTitle, "\\/", false);

		FolderItem parent = m_control.getFolder();
		Item currentItem = null;

		while( stoke.hasMoreElements() )
		{
			// find item in the list
			if((currentItem = parent.findItem( stoke.nextToken() )) != null ) 
			{
				// if more elements to come
				if( stoke.hasMoreElements() )
				{
					if( currentItem instanceof FolderItem )
					{
						// if more elements make parent currentItem
						parent = (FolderItem)currentItem;
					}	
					else // error we are not a folder and we need one 
					{	
						return ; // return not found 	
					}
				} 
			}
			else 
			{
				return; 
			}// end else not found
		
		} // end while tokens

		// and remove it
		if( parent != null && currentItem != null )
		{
			parent.removeItem( currentItem );
		}

		m_control.setFolder( m_control.getFolder() );
	}

	// highlight a particular item
	public void highlight( String itemTitle )
	{
		itemTitle = getArgument( itemTitle, 0 );
		itemTitle.trim();
		m_control.highlight( itemTitle );
	}

	// hightlight next visible item
	public void next( )
	{
		m_control.next( true );
	}

	// highlight previous visible item
	public void previous( )
	{
		m_control.prev( true );
	}
	
	// activate current selection
	public void activate( )
	{
		m_control.activate( );
	}

	public void run()
	{
		// setup base URL
		String param = getParameter("baseURL");
		
		// setup base URL
		try{
			if( param != null )
			{
				m_baseURL = new URL( param );
			}
			// default base url
			else m_baseURL = getDocumentBase();
		} 	
		catch( MalformedURLException e ) 
			{m_baseURL = getDocumentBase();}


		// set root item title
		param = getParameter("rootTitle");
		if (param != null) 
			addItem( param, null, this, m_baseURL, true );

		Item statusItem = new Item( "Loading items..");
		
		m_control.m_folder.setTitle( m_folder.getTitle() );
		m_control.m_folder.addElement( statusItem );
		m_control.initalise( );
		
		// readin the item params item 1 to item N
		int x = 1;					
		while(( param = getParameter( "item" + x++)) != null )
		{
			if( param.startsWith("**") ) continue;
			addItem( param, null,  this, m_baseURL, false );
		}

		statusItem.setTitle( "Loading images..." );
		m_control.initalise( );

		// load images
		MediaTracker tracker = new MediaTracker( this );

		// pull all the images our of the hashtable
		// and add them to the tracker
		Enumeration en = m_imageList.elements();
		while( en.hasMoreElements() )
		{
			tracker.addImage( (Image)en.nextElement(), 0 );			
		}

		// load all the images
		try
		{
			tracker.waitForAll();
		}
		catch( InterruptedException e )
		{
			// do nothing as compontents will handle image problems
		}

		m_folder = m_control.setFolder( m_folder );	
	}


	// test if image is already to be loaded
	// or add to load list
	private final Image addImage( String imageURL, Applet applet )
	{
		// dont bother with 0 length strings
		if( imageURL.length() == 0 ) return null;

		// check if we already have this loaded
		if( m_imageList.containsKey( imageURL ) )
		{
			return (Image) m_imageList.get( imageURL );
		}
		else // create image and add it to the list
		{
			try
			{
				URL link = new URL( applet.getCodeBase(), imageURL );
							
				//Image image = applet.getImage( applet.getDocumentBase(), imageURL );
				Image image = applet.getImage( link );
				m_imageList.put( imageURL, image ); 

				return image;
			}
			catch( MalformedURLException e )
			{ 
				// do nothting 
			}
		}
		return null;
	}

	// parse image arguments and add images to the item
	private final Item addImage( Item item, String imageArgs, Applet applet )
	{
		if( imageArgs.length() == 0 ) return null;

		StringTokenizer stoke = new StringTokenizer(imageArgs, ",", false );

		if( stoke.hasMoreElements() )
		{
			item.m_icon = ( addImage( stoke.nextToken(), applet ) );

			// add second image if its a folder
			if( stoke.hasMoreElements() )
			{
				item.m_highlightIcon = ( addImage( stoke.nextToken(), applet ) );
			}
		}
		return item;
	}

	private final String addItem( String itemTitle, String none, Applet applet, URL baseURL, boolean rootItem )
	{
		Item item=null;

		// first item == item text and depth
		if( rootItem ) // if root directory item
		{
			item = m_folder;	// item equals root
			item.setTitle(  getArgument(itemTitle,0) );
		}
		else // normal item
		{
			if(( item = addItemString(  getArgument(itemTitle,0) )) == null ) 
				return null;
		}
	
		// second item == image
		addImage( item,  getArgument(itemTitle,1) , applet );

		ItemAction action  = new ItemAction( this );
		item.setAction( action );

		// forth argument is the status string
		action.setStatus(  getArgument(itemTitle,3) );

		// third item == link url 
		String linkArgs = getArgument(itemTitle,2);

		if( linkArgs.length() != 0 )
		{
			StringTokenizer stoke = new StringTokenizer(linkArgs, ",", false );
		
			String link = stoke.nextToken();
			String targetFrame = "_top";
			
			// strip frame from string
			if( stoke.hasMoreElements() )
				targetFrame = stoke.nextToken();
			
			try
			{
				URL linkURL = new URL( baseURL, link );
				action.setDocument( linkURL, targetFrame );
			}
			catch( MalformedURLException e )
			{ 
				action.setStatus( "Error: Malformed URL Exception" );
			}
		}
		
		return item.getTitle();
	}

	// strip an argument from an argument string broken up by ;
	private final String getArgument( String args, int arg_number )
	{
		int position=0;
		int arg_position =0;
		
		try
		{
			// count across argument breakers ';'
			while( arg_position < arg_number && position < args.length() )
			{
				if( args.charAt( position++ ) == ';' )
					arg_position++;
			}

			// eat and white leading white space
			while( args.charAt( position ) == ' ' && position < args.length() )
					position++;
			
			if( arg_position != arg_number ) return "";

			int arg_start = position;
		
			// cut out our argument
			while( position < args.length() && args.charAt( position ) != ';' )
				position++;

			return args.substring( arg_start, position );

		}
		catch( StringIndexOutOfBoundsException e )
		{
			return "";
		}
	}

    private final String unescape(String s, StringBuffer buf)
    {
	if(s.indexOf('#') != -1) {
	  int i = 0;
	  int len = s.length();
	  buf.setLength(0);
	  while(i<len) {
	      char c = s.charAt(i++);
	      if(c == '#') {
		  int n = 0;
		  while(i < len && Character.isDigit(c = s.charAt(i))) {
		      n = n * 10 + Character.digit(c, 10);
		      ++i;
		  }
		  if (c == '#')
		      ++i;
		  c = (char) n;
	      }
	      buf.append(c);
	  }
	  return buf.toString();
	} else {
	  return s;
	}
    }
    
	private final Item addItemString( String itemTitle )
	{
		if( itemTitle.length() == 0 ) return null; 

		// break string down into elements
		StringTokenizer stoke = new StringTokenizer(itemTitle, "\\/", false);

		FolderItem parent = m_folder;
		Item currentItem = null;
		StringBuffer buf = new StringBuffer();

		while( stoke.hasMoreElements() )
		{
			String token = unescape(stoke.nextToken(), buf);
			
			// find item in the list
			currentItem = parent.findItem( token );

			// no item create one
			if( currentItem == null )
			{
				if( stoke.hasMoreElements() )
				{	currentItem = (Item)new FolderItem( token ); }
				else
				{	currentItem = new Item( token ); }
						
				parent.addElement( currentItem );
			}
			else // if found item 
			{
				// if more elements to come
				if( stoke.hasMoreElements() )
				{
					// if we are not a folder convert item to a folder
					if( !(currentItem instanceof FolderItem) )
					{
						Item converted = (Item) new FolderItem(currentItem );
						parent.replaceItem( currentItem, converted );
						currentItem = converted;
					}
				}
			} // end else found

			// if more elements make parent currentItem
			if( stoke.hasMoreElements() )
					parent = (FolderItem)currentItem;
		} // end while tokens

		return currentItem;
	}

}
