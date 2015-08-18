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


import java.awt.*;
import java.util.Hashtable;
import java.awt.image.ImageObserver;
import java.applet.Applet;
import java.util.Enumeration;
import java.util.StringTokenizer;
import java.net.URL;
import java.net.MalformedURLException;

public class TreeControl extends ScrPane 
{
	FolderItem m_folder;
	Item m_selected;
	Item m_mouseHit;
	int m_activation_type;

	int m_layoutOffsetX;
	int m_layoutOffsetY;

	public Color m_detailColour;    // directory lines color  
	public Color m_highColour;  // hightlight background colour
	public Color m_textColour;	 // text colour	
	public Color m_highTextColour; // text highligh colour

	public TreeControl()
	{
		m_folder = new FolderItem("Directory");
		m_selected = m_folder;

		m_activation_type = 0;

		// setup default colors
		m_detailColour = Color.lightGray;
		m_highColour = new Color(153,0,66); // rose red
		m_textColour = Color.black;
		m_highTextColour = Color.white;

		m_layoutOffsetX = 1;
		m_layoutOffsetY = 1;
	
		setBackground( m_bgColour );
	}

	// set layout offset for the directory structure
	public void setOffset( int x, int y )
	{
		m_layoutOffsetX = x;
		m_layoutOffsetY = y;
	}

	public void draw()
	{
		Rectangle rect  = m_folder.layout( m_layoutOffsetX, m_layoutOffsetY );
		setSize( rect.x + rect.width, rect.y + rect.height );

		Graphics g = getCanvas();
	
		clearRect( g );
		m_folder.draw( g, this, Item.DRAW_FULL );

		if( m_selected != null ) 
			m_selected.draw( g, this, Item.DRAW_HIGHLIGHT );
		
		refresh();
	}

	public FolderItem getFolder()
	{
		return m_folder;	
	}

	public boolean mouseDown( Event e, int x, int y )
	{
		if((m_mouseHit = m_folder.hitTest( x + m_offsetx ,y + m_offsety)) != null )
		{
			if( m_mouseHit != null ) 
				m_mouseHit.draw( getCanvas(), this, Item.DRAW_HIGHLIGHT );

			// catch double clicks
			if( e.clickCount > 1 ) 
				m_activation_type = Item.ACT_DBCLICK;
			else 
				m_activation_type = Item.ACT_CLICK;

			refresh();
		}
		
		return true;
	}

	public FolderItem setFolder( FolderItem folder )
	{
		FolderItem base = m_folder;
		m_folder = folder;
		
		m_selected = null;
		m_mouseHit = null;
		m_activation_type = 0;

		m_selected = m_folder;
		initalise();	
		
		return base;
	}

	public void initalise( )
	{
		// load all the images
		Font font = getFont(); 
		if( font == null )
		{
			if( (font = new Font( "Dialog", 0 ,10)) != null )
				setFont( font );
		}

		// set the background color if changed
		m_folder.expand( true );

		// setup all the components
		m_folder.initalise( getFontMetrics( font ) , this );
	
		draw();
	}


	// find item from string eg "folder\subdir\item"
	// and set item as selected
	// returns null if not found else the item that was selected
	public Item highlight( String itemTitle )
	{
		// break string down into elements
		StringTokenizer stoke = new StringTokenizer(itemTitle, "\\/", false);

		FolderItem parent = m_folder;
		Item currentItem = null;
		boolean redrawFlag = false; // redraw flag

		while( stoke.hasMoreElements() )
		{
			if( !parent.isExpanded() )
			{
				// expand folder
				parent.expand( true );
				redrawFlag = true;
			}
			
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
						// validate any changes
						if(	redrawFlag ) draw();
						
						return null; // return not found 	
					}
				} 
			}
			else 
			{
				// validate any changes
				if(	redrawFlag ) draw();
				
				// error no item by that name return not found
				return null; 
			}// end else not found
		
		} // end while tokens

		// redraw screen as we have changed the number of visible items
		if(	redrawFlag ) draw();
		// and set this item as selected
		setSelected( currentItem );
				
		return currentItem;
	}

	// set an item as selected
	public void setSelected( Item item )
	{
		if( m_selected != item )
		{
			Graphics g = getCanvas();

			item.draw( g, this, Item.DRAW_HIGHLIGHT );
			
			if( m_selected != null )
			{
				m_selected.draw( getCanvas(), this, Item.DRAW_CLEAR );
			}

			m_selected = item;

			if(!makeVisible( item.getRect() ) )	
				refresh();

			// activate item
			m_selected.activate( Item.ACT_HIGHLIGHT, null  );
		}
	}

	public boolean mouseUp( Event e, int x, int y )
	{
		if( m_mouseHit != null )
		{
			Item item = m_folder.hitTest( x+m_offsetx , y+m_offsety);

			if( item == m_mouseHit )
			{	
				e.x += m_offsetx;
				e.y += m_offsety;

				// setup activation 
				if( e.clickCount > 1 ) m_activation_type = Item.ACT_DBCLICK;

				// acitvate item
				if( item.activate( m_activation_type, e ) )
				{
					m_selected = item;
					draw();				
					makeVisible( item.getRect() );	
				}
				else
				{
					setSelected( item );
				}

				// clear activation
				m_activation_type = Item.ACT_CLICK;
				m_mouseHit = null;
			}
			else
			{ // clear mouse hit item
				Graphics g = getCanvas();
				m_mouseHit.draw( g, this, Item.DRAW_CLEAR );
				refresh();
			}
		}
		return true;
	}

	public boolean mouseMove( Event e, int x, int y )
	{
		Item item = null;
			
		if((item = m_folder.hitTest( x+m_offsetx , y+m_offsety)) != null )
		{
			item.activate( Item.ACT_HIGHLIGHT, null  );
		}

		return true;
	}

	public void next( boolean expand )
	{
		Item next = null;

		if( (next = m_folder.getNext( m_selected, expand )) != null )
		{
			if( expand ) draw();
			setSelected( next );
		}
	}

	public void activate( )
	{
		if( m_selected != null )
			m_selected.activate( Item.ACT_CLICK, null );			
	}


	public void prev( boolean expand )
	{
		Item next = null;

		if( (next = m_folder.getPrev( m_selected, expand )) != null )
		{
			if( expand ) draw();
			setSelected( next );
		}
	}

	public boolean keyDown(Event  evt, int  key)
	{
		
		Item next = null;
		// up arrow previous item
		if( key == Event.UP ) 
		{
			prev( false );		return false;
		}

		// down arrow select next item
		if( key == Event.DOWN ) 
		{
			next( false );		return false;
		}

		// enter of right arrow activate item
		if( key == 10 || key == Event.RIGHT ) // enter
		{
			if( m_selected.activate( Item.ACT_DBCLICK, null ) )
			{
				draw();	
				makeVisible( m_selected.getRect() );	
			}
			return false;
		}

		// left arrow close up folder
		if( key == Event.LEFT )
		{
			if( m_selected instanceof FolderItem )
			{
				FolderItem item = (FolderItem)m_selected;
				
				if( item.isExpanded() )
				{
					item.expand( false );
					draw();				
				}
			}
			return false;
		}

		return true;
	}

}