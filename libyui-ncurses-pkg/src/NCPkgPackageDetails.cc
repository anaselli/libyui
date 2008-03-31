/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       NCPkgPackageDetails.cc

/-*/
#define YUILogComponent "ncurses-pkg"
#include <YUILog.h>

#include "NCPkgTable.h"
#include "NCPkgPackageDetails.h"

/*
  Textdomain "ncurses-pkg"
*/

NCPkgPackageDetails::NCPkgPackageDetails ( YWidget *parent, string initial_text, NCPackageSelector *pkger)
    : NCRichText (parent, initial_text)
    , pkg (pkger)
{

}

string NCPkgPackageDetails::createRelLine( const zypp::Capabilities & info )
{
    string text = "";
    zypp::Capabilities::const_iterator
	b = info.begin (),
	e = info.end (),
	it;
    unsigned int i, n = info.size();

    for ( it = b, i = 0; it != e; ++it, ++i )
    {
	text = text + (*it).asString();
	if ( i < n - 1 )
	{
	    text = text + ", ";
	}
    }

    return text;
}

string NCPkgPackageDetails::createText( list<string> info, bool oneline )
{
    list<string>::iterator it;
    string text = "";
    unsigned int i;

    for ( i = 0, it = info.begin(); it != info.end() && i < 1000; ++it, i++ )
    {
	text += (*it);
	if ( i < info.size()-1 )
	{
	    if ( oneline && i < 999 )
	    {
		text += ", ";
	    }
	    else
	    {
		text += "<br>";
	    }
	}
	if ( i == 999 )
	{
	    text += "...";
	}
    }

    return text;
}

string NCPkgPackageDetails::commonHeader( ZyppObj pkgPtr )
{
   string text;
   text += "<h3>" + pkgPtr->name() + " - "; 
   text += pkgPtr->summary() + "</h3>";

   return text;
}

void NCPkgPackageDetails::longDescription ( ZyppObj pkgPtr )
{
   string text = "";  
  
   //text += commonHeader( pkgPtr );
   //text += pkgPtr->description();

   // show the description
   setValue( text );
}

void NCPkgPackageDetails::technicalData( ZyppObj pkgPtr, ZyppSel slbPtr )
{
    string instVersion = "";
    string version = "";
    string text = "";
    
    text += commonHeader( pkgPtr );
    //text += " - ";
    
    //text += pkgPtr->summary();
    
    if ( slbPtr->hasBothObjects () )
    {
        ZyppObj io = slbPtr->installedObj ();
        instVersion = io->edition().version();
        instVersion += "-";
        instVersion += io->edition().release();
        ZyppObj co = slbPtr->candidateObj ();
        version = co->edition().version();
        version += "-";
        version += co->edition().release();
    }
    else
    {
        version = pkgPtr->edition().version();
        version += "-";
        version += pkgPtr->edition().release();
    }
    
    text += NCPkgStrings::Version();
    text +=  version;
    if ( instVersion != "" )
    {
        text += "  ";
        text += NCPkgStrings::InstVersion();
        text += instVersion;
    }
    text +=  "<br>";
    
    // show the size
    text += NCPkgStrings::Size();
    text += pkgPtr->size().asString();
    text +=  "  ";
    
    ZyppPkg package = tryCastToZyppPkg (pkgPtr);
    if ( package )
    {
        // add the media nr
        text += NCPkgStrings::MediaNo();
        char num[5];
        int medianr = package->mediaNr ();
        sprintf( num, "%d", medianr );
        text += num;
        text += "<br>";
    
        // the license
        text += NCPkgStrings::License();
        text += package->license();
        text += "  ";
        text += "<br>";
    
        // the rpm group
        text += NCPkgStrings::RpmGroup();
        text += package->group ();
        text += "<br>";
	
	//authors, in one line
        text += NCPkgStrings::Authors();
        list<string> authors = package->authors(); // zypp::Package
        text += createText( authors, true );

    }

    setValue (text);

}

void NCPkgPackageDetails::fileList( ZyppSel slbPtr )
{
   string text = "";
   // the file list is available only for installed packages
   ZyppPkg package = tryCastToZyppPkg (slbPtr->installedObj());
   
   if ( package )
   {
       text += commonHeader( slbPtr->theObj() ); 
       text += NCPkgStrings::ListOfFiles();
       // get the file list from the package manager/show the list
       list<string> fileList = package->filenames();
       text += createText( fileList, false ) ;
   }

   else 
	text = _("<i>This information is available for installed packages only</i>");

   setValue(text);
}

void NCPkgPackageDetails::dependencyList( ZyppObj pkgPtr, ZyppSel slbPtr )
{
    string text = commonHeader( pkgPtr );    
    // show the relations, all of them except provides which is above
    zypp::Dep deptypes[] = {
	zypp::Dep::PROVIDES,
        zypp::Dep::PREREQUIRES,
        zypp::Dep::REQUIRES,
        zypp::Dep::CONFLICTS,
        zypp::Dep::OBSOLETES,
        zypp::Dep::RECOMMENDS,
        zypp::Dep::SUGGESTS,
        zypp::Dep::FRESHENS,
        zypp::Dep::ENHANCES,
        zypp::Dep::SUPPLEMENTS,
    };
    for (size_t i = 0; i < sizeof (deptypes)/sizeof(deptypes[0]); ++i)
    {
        zypp::Dep deptype = deptypes[i];
        zypp::Capabilities relations = pkgPtr->dep (deptype);
        string relline = createRelLine (relations);
        if (!relline.empty ())
        {
    	// FIXME: translate
    	text += "<b>" + deptype.asString () + ": </b>"
    	    + relline + "<br>";
        }
    }

    setValue (text);
    
}
