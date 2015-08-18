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
import java.awt.image.ImageObserver;
import java.util.Vector;
import java.util.Enumeration;

public class FolderItem  extends Item  
{
	public static boolean m_drawDotted = false;
	int TABSTOP = 20;
	int INDENT = 4;
	int m_half_TextHeight = 10; 

	//static final int EXPAND_COLLAPSE_WIDTH = 15;

	Rectangle m_rect;	// current drawing area
	boolean m_expanded;

	Vector m_list;

	////////////////////////////////////////////////
	// construction

	// 	public FolderItem( String title )
	//	costruct new item with title of title	
	public FolderItem( String title )
	{
		super(title);
		
		m_expanded = false;
		m_rect = new Rectangle();
		m_list = new Vector();
	}

	// 	public FolderItem( Item item )
	// copy items contents and construct object
	public FolderItem( Item item )
	{
		super( item );

		m_expanded = false;
		m_rect = new Rectangle();
		m_list = new Vector();
	}

	// 	public FolderItem( FolderItem item )
	// copy constructor 

	public FolderItem( FolderItem item )
	{
		super( (Item) item );

		m_expanded = item.isExpanded();
		m_rect = new Rectangle();
		m_list = new Vector();

		// copy element refereces across
		Enumeration en = item.elements();	

		while( en.hasMoreElements() )
		{
			addElement( en.nextElement() );			
		}
	}

	//////////////////////////////////////////////////
	// list operations
	public Enumeration elements()
	{
		return m_list.elements();
	}

	public void addElement( Object item )
	{
		m_list.addElement( item );
	}

	public boolean isEmpty( )
	{
		return m_list.isEmpty();
	}

	//////////////////////////////////////////////////
	//	public void draw( Graphics g, ImageObserver ob, int highlight )
	// draw ourselves onto the screen given a graphics 
	public void draw( Graphics g, TreeControl ob, int highlight )
	{
		if( !isExpanded() || (highlight & DRAW_FULL) == 0 ) 
		{
			if( isExpanded() ) highlight |= DRAW_SECONDIMAGE;

			super.draw(g, ob, highlight);
			return;
		}
		else // redraw children
		{
			// start at the top
			super.draw(g, ob, highlight | DRAW_SECONDIMAGE);
			
			int x = INDENT + m_rect.x;
		//	int half_TextHeight = super.getHeight() /2;
			int lasty = super.getHeight() + m_rect.y;
			int y = lasty;// + half_TextHeight;

			// shift lasty up to the bottom of the icon
/*			if( super.getIcon() != null )
			{
				// make sure we have an image hieght
				int image_height = (super.getIcon()).getHeight( ob); 
			
				if( image_height > 0 )
					lasty -= (super.getHeight() - image_height)/2; 
			}

*/			Enumeration en = elements();	

			// travel list of items
			while( en.hasMoreElements() )
			{
				Item element = (Item) en.nextElement();			
				element.draw(g, ob, highlight);

				// draw item lines
				Color color = g.getColor();
				g.setColor( ob.m_detailColour );

				// draw expand collapse box
				if( element instanceof FolderItem )
				{
					FolderItem fitem = (FolderItem) element;

					y = (fitem.getRect()).y + (fitem.getTitleRect().height/2);
					
					y -= 4;
					int offsetx = x - 4;


					// draw box
					g.drawRect( offsetx, y , 8 , 8 );	

					// add expand collapse box minus
					g.drawLine( offsetx +2 , y + 4, offsetx +6,  y+4 );
					
					// draw plus if needed	
					if( !fitem.isExpanded() )
						g.drawLine( offsetx +4 , y + 2, offsetx +4,  y+6 );

					// add line across to the image
					drawLineAcross(g, offsetx + 8 , y + 4, x+(TABSTOP-INDENT) );

					// draw line down to the top of component
					drawLineDown(g, x, lasty, y );

					// shift down
					lasty = y + 8;
				}
				else
				{
					y = (element.getRect()).y + (element.getHeight())/2;

					drawLineDown(g, x, lasty, y );
					drawLineAcross(g, x,y, x+(TABSTOP-INDENT) );
					
					lasty = y;
				}

				g.setColor( color );
			}
		}
	
	}

	// draw dotted line
	void drawLineAcross(Graphics g, int xtop, int ytop, int xbot )
	{
		if( m_drawDotted )
		{
			while( xtop < xbot )
			{
				g.drawLine( xtop, ytop, xtop, ytop );
				xtop+=2;
			}
		}
		else g.drawLine( xtop, ytop, xbot, ytop );
	}

	// draw dotted line
	void drawLineDown(Graphics g, int xtop, int ytop, int ybot )
	{
		if( m_drawDotted )
		{
			while( ytop < ybot )
			{
				g.drawLine( xtop, ytop, xtop, ytop );
				ytop+=2;
			}
		}
		else 	g.drawLine( xtop, ytop, xtop, ybot);
	}


	////////////////////////////////////////////////////////
	// 	public boolean activate( int activate_type, Event event )
	// activate item with some form of action input
	// uses event co-ordinates to test if the expand collapse box
	// has been hit or the item send null if not a user input
	//
	// returns true if item's layout has changed and needs a full redraw
	public boolean activate( int activate_type, Event event )
	{
		if( event != null )
		{
			// if hit title text activate item link else
			// must have hit the expand collaspe box
			if(	(super.getRect()).inside( event.x, event.y ) )
			{
				super.activate( activate_type, null );
			}
			else
			{
				expand( !m_expanded );
				return true;
			}
		}			
		else // defualt activte super
		{
				super.activate( activate_type, null );
		}

		// default expand
		if( /*activate_type == ACT_CLICK ||*/ activate_type == ACT_DBCLICK )
		{
			expand( !m_expanded );
			return true; // redraw
		}
		
		return false;	// nothing changed no redraw
	}

	////////////////////////////////////////////////////////
	// 	public Item hitTest( int x, int y )
	// does x,y fall within the rectangle
	// returns the hit item this ,its child or null
	public Item hitTest( int x, int y )
	{
		Item item;

		// test ourselves
		if( !m_rect.inside( x,y ) ) return null;
		
		// test our title
		if( super.hitTest( x,y ) != null )
			return this;

		// setup rectangle for expand collaspe box testing	
		Rectangle expand_rect = new Rectangle( 10,10);
		expand_rect.x = m_rect.x + INDENT - 4;

		// test all of our children
		Enumeration en = elements();

		while( en.hasMoreElements() )
		{
			Item element = (Item) en.nextElement();			
			
			if( (item = element.hitTest( x,y )) != null )
				return item;

			// if folder item expand search to include 
			// expand collapse box on the left of an item
			if( element instanceof FolderItem )
			{
				FolderItem fitem = (FolderItem)element;

				// get items rectangle
				Rectangle rect = fitem.getTitleRect();
				// shift rect to the left of this item
				//expand_rect.y = rect.y + (rect.height/2) - 4;
				//expand_rect.y = rect.y + m_half_TextHeight -4;
				expand_rect.y = (fitem.getRect()).y + (fitem.getTitleRect().height/2)-4;


				// and hit test
				if( expand_rect.inside( x,y ) == true )
					return element;
			}
		}
		
		return null; // not found
	}

	////////////////////////////////////////////////////////
	// 	public void initalise( FontMetrics fm, ImageObserver ob )
	// intalise drawing area this should be called after construction and 
	// after any changes to title or icon. This sets up and
	public void initalise( FontMetrics fm, ImageObserver ob )
	{
		super.initalise( fm, ob );	// set top leave item
		
		// work out indent from super
		if( getIcon() != null )
		{
			INDENT = getIcon().getWidth( ob )/2;
			if( INDENT < 1 ) INDENT = 4;
			if( INDENT > 15 ) INDENT = 15;

			TABSTOP = INDENT + 10;	
		}
	
		m_half_TextHeight =  (fm.getLeading() + fm.getMaxAscent() + fm.getMaxDescent())/2; 

		// set rect for all children
		Enumeration en = elements();	

		while( en.hasMoreElements() )
		{
			Item element = (Item) en.nextElement();			
			element.initalise( fm, ob );
		}
	}

	////////////////////////////////////////////////////////
	// public Rectangle layout( int x, int y )
	// layout items of this item and return the bounding rectangle
	// int x, y - position to start laying out from ( top left )
	// returns Rectangle  - bounding rectangle of this item and children
	public Rectangle layout( int x, int y )
	{
		// if has no children at present hand back just the title bar's rectangle
		if( !isExpanded() ) 
		{
			Rectangle rect = super.layout(x,y);
			
			// copy rectangle 
			m_rect.x = rect.x;
			m_rect.y = rect.y;
			m_rect.width  = rect.width;
			m_rect.height = rect.height;
			
			// and return it
			return m_rect;
		}
		else // else layout children
		{
			m_rect.x = x;
			m_rect.y = y;

			// start with our title
			Rectangle rect = super.layout(x,y); 
			int width  = rect.width;
			int height = rect.height;

			// add folder item indent
			x += TABSTOP;

			Enumeration en = elements();	

			// travel list of items
			while( en.hasMoreElements() )
			{
				Item element = (Item) en.nextElement();			
				rect = element.layout(x, y + height);
					
				// get the width of the widest item
				if( rect.width > width ) width = rect.width;
		
				// add to hieght 
				height += rect.height;
			}
				
			// set our rectangle to contain all children
			m_rect.width = width + TABSTOP;
			m_rect.height = height;
				
			// and return
			return m_rect;
		}
	}

	////////////////////////////////////////////////////////
	// public Rectangle getRect( )
	// get the drawing area and bounding rectangle of this item 
	public Rectangle getRect( )
	{
		return m_rect;
	}


	////////////////////////////////////////////////////////
	// public Rectangle getRect( )
	// get the drawing area and bounding rectangle of this item 
	public Rectangle getTitleRect( )
	{
		return super.getRect();
	}

	////////////////////////////////////////////////////////
	// public boolean isExpanded( )
	// test if folder is expanded eg children visible
	public boolean isExpanded( )
	{
		return m_expanded;
	}

	////////////////////////////////////////////////////////
	// 	public void expand( boolean expand )
	// set expanded of colapsed state for folder
	public void expand( boolean expand )
	{
		m_expanded = expand;

		// if we are closing up this folder
		// we need to close up all child folders too
		if( expand == false )
		{
			Enumeration en = elements();	

			// travel list of items
			while( en.hasMoreElements() )
			{
				Item element = (Item) en.nextElement();			
				
				if( element instanceof FolderItem )
					((FolderItem)element).expand( false );
			}
		}
	}

	////////////////////////////////////////////////////////
	// 	public int getWidth() 
	// get the current width of object
	public int getWidth() 
	{ 
		return m_rect.width; 
	}

	////////////////////////////////////////////////////////
	// 	public int getHeight() 
	// get the current height of object
	public int getHeight() 
	{ 
		return m_rect.height; 
	}

	////////////////////////////////////////////////////////
	// 	public Item findItem( String title )
	// recursivly finds item given its title eg folder/child_folder/item
	// reutrns null if not found
	public Item findItem( String title )
	{
		if( title == null ) return null; // fail safe test
		
		Enumeration en = elements();	

		// travel list of items
		while( en.hasMoreElements() )
		{
			Item element = (Item) en.nextElement();			
			
			if( title.equalsIgnoreCase( element.getTitle() ))
				return element;
		}
		return null; // not found
	}

	////////////////////////////////////////////////////////
	// public Item removeItem( Item item )
	// and removes the item and its children from the tree
	// reutrns null if not found
	public Item removeItem( Item item )
	{
		if( m_list.removeElement( item ) == true )
		{	return item; }
		else{ return null; }
	}

	////////////////////////////////////////////////////////
	// 	public Item replaceItem( Item item, Item replace )
	//  searches imediate children for item and replaces it with the one given
	// reutrns null if not found else the original replaced Item
	public Item replaceItem( Item item, Item replace )
	{
		int index;
		if(( index = m_list.indexOf(item)) != -1 )
		{
			m_list.setElementAt( replace, index );
			return item;		
		}
		else {	return null; }	// not found 
	}

	////////////////////////////////////////////////////////
	//	protected Item getPrev( int index, boolean expand )
	// get previous item given list index
	// proctected helper function for getPrev( Item ) 
	protected Item getPrev( int index, boolean expand )
	{
		// if first element then return this
		if( index <= 0 ) return this;

		// else find item from index
		if( index-1 >= 0 )
		{
			Item item;
			try	{ item = (Item)m_list.elementAt( index-1 ); }
			// catch any error 
			catch ( ArrayIndexOutOfBoundsException e ){ item = null; }

			// if item is a  folder then get the last item of the folder
			if( item != null && item instanceof FolderItem )
				item = ((FolderItem)item).getLastItem( expand );
			
			return item; // and return
		}
		else{ return null; } // error no such item
	}

	////////////////////////////////////////////////////////
	// protected Item getNext( int index, boolean expand )
	// get next item given index
	// protected helper function for getNext( Item )
	// boolean expand - will expand folder and children to find last item
	protected Item getNext( int index, boolean expand )
	{
		if( expand ) expand( expand );

		// test current item is a folder  
		Item currItem;
		try	{ currItem = (Item)m_list.elementAt( index ); }
		// catch any error and return the 
		catch ( ArrayIndexOutOfBoundsException e ){ currItem = null; }
		
		if( currItem instanceof FolderItem )
		{
			// if so get the next top item
			FolderItem fi = (FolderItem)currItem;
			Item next = fi.getNext( currItem, expand );

			// if no top item fall through
			if( next != currItem ) return next;
		}

		if( m_list.size() > index+1 )
		{
			Item item;
			try	{ item = (Item)m_list.elementAt( index+1 ); }
			// catch any error and return the 
			catch ( ArrayIndexOutOfBoundsException e ){ item = null; }

			return item;
		}

		// found end of this list
		else return null;
	}

	////////////////////////////////////////////////////////
	// 	public Item getLastItem( boolean expand )
	// return the last item in the list
	// returns this if no child items
	// boolean expand - will expand folder and children to find last item
	public Item getLastItem( boolean expand )
	{
		if( (!isExpanded()  && !expand ) || isEmpty()) return this;
		else
		{
			if( expand ) expand( expand );

			Item item = (Item) m_list.lastElement();	
			if( item instanceof FolderItem ) 
				return ((FolderItem)item).getLastItem( expand );
			else return item;
		}
	}

	////////////////////////////////////////////////////////
	// 	public Item getFirstItem( boolean expand )
	// return the first Item in the list
	// returns this is no child items
	// boolean expand - will expand folder and children to find last item
	public Item getFirstItem( boolean expand )
	{
		if( (!isExpanded() && !expand) || isEmpty()  ) return this;
		else
		{
			if( expand ) expand( expand );

			return (Item) m_list.firstElement();	
		}
	}

	////////////////////////////////////////////////////////
	// 	public Item getNext( Item item, boolean expand )
	// get next availble item in tree structure
	// this will return the item if found
	// null if not found and the passed item
	// if found item but its at the end of the tree
	// boolean expand - will expand folder and children to find last item
	public Item getNext( Item item, boolean expand )
	{
		if( item == this ) return getFirstItem( expand );
		
		// if we arnt expanded then nothing to get
		if( (!isExpanded() && !expand ) || isEmpty() ) return null;
		
		int item_index =0;

		// is it part of local level list
		if( (item_index = m_list.indexOf( item )) != -1 )
		{
			Item next = getNext(item_index, expand);
			
			// if error return found end of list
			if( next == null ) next = item;
			
			return next;
		}
		else // search all child folders
		{
			Enumeration en = elements();	

			// travel list of items and ask each child
			while( en.hasMoreElements() )
			{
				Item element = (Item) en.nextElement();			
				if( element instanceof FolderItem )
				{
					FolderItem fi = (FolderItem)element;
					Item found = fi.getNext( item, expand );

					if( found != null && expand ) expand( expand );

					// if found == item then return next in list
					if( found == item )
					{
						if( en.hasMoreElements() )
						{
							return (Item)en.nextElement();
						}
						else return item;
					}
					else if( found != null ) return found;

				} // end if folder
			} // end while items
		}// end else search all children
		
		// reached end of list and exhausted all children
		return null;
	}

	////////////////////////////////////////////////////////
	// 	public Item getPrev( Item item, boolean expand )
	// get the previous item from the one given
	// this will return the item if found
	// null if not found and the passed item
	// if found item but its at the top of the tree
	// boolean expand - will expand folder and children to find last item
	public Item getPrev( Item item, boolean expand )
	{
		if( item == this ) return item;
		
		// if we arnt expanded then we need not look
		if( (!isExpanded() && !expand) || isEmpty() ) return null;
		
		// is it part of local level list
		int item_index = m_list.indexOf( item );

		// is it part of local level list
		if( item_index != -1 )
		{
			return getPrev( item_index, expand );
		}
		else // search all child folders
		{
			Enumeration en = elements();	

			// travel list of items and ask each child
			while( en.hasMoreElements() )
			{
				Item element = (Item) en.nextElement();			
				if( element instanceof FolderItem )
				{
					FolderItem fi = (FolderItem)element;
		
					Item found = fi.getPrev( item, expand );
					
					// find previous item in the list and select 
					if( found == item )
					{
						return getPrev( m_list.indexOf( fi ), expand );
					}
					else if( found != null )
						{	
							if( expand ) expand( expand );
							return found;
						}	
				}// end if folder
			}// end while children
		}// end else search children

		// reached end of list and exhausted all children
		return null;
	}

/* unused constructors
	public FolderItem( String title, Image icon )
	{
		super( title, icon );
		m_expanded = false;
		m_rect = new Rectangle();
		m_list = new Vector();
	}

	public FolderItem( String title, Image icon, boolean expand )
	{
		super( title, icon );

		m_expanded = expand;
		m_rect = new Rectangle();
		m_list = new Vector();
	}
*/

} // end folder item