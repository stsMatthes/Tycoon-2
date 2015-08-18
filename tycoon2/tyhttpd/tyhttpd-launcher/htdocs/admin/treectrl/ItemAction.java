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
import java.net.*;

public class ItemAction extends java.lang.Object 
{
	String m_status;
	URL m_doc;
	String m_docTarget;
	public static Applet m_applet;
	public static boolean m_showURL = true;

	///////////////////////////////////////////////////////////////
	// initalisation 

	public ItemAction(Applet applet )
	{
		m_applet = applet;
	}

	// set status string and add URL link to the end if present
	public void setStatus( String status )
	{
		// copy status sting
		if( status != null )
			m_status = new String( status );

		// and add url if present
		if( m_doc != null && m_showURL )
		{
			if( m_status == null ) m_status = new String();
			else m_status = m_status.concat(" - ");
			
			m_status = m_status.concat( m_doc.toString() );
		}	
	}

	public void setDocument( URL doc, String target )
	{
		m_doc = doc;
		m_docTarget = target;

		setStatus( m_status );
	}

	////////////////////////////////////////////////////////////////////
	// actavtion actions
	public void showStatus( )
	{
		if( m_status != null ) 
			m_applet.showStatus( m_status );
 	}
	
	public void  showDocument( )
	{
		if( m_doc != null )
		{
			AppletContext ac = m_applet.getAppletContext();
			ac.showDocument( m_doc, m_docTarget );
		}
	}
}