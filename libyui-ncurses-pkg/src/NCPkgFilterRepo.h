/*
  Copyright (c) [2002-2011] Novell, Inc.
  Copyright (c) 2021 SUSE LLC

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) version 3.0 of the License. This library
  is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
  License for more details. You should have received a copy of the GNU
  Lesser General Public License along with this library; if not, write
  to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
  Floor, Boston, MA 02110-1301 USA


  File:       NCPkgFilterRepo.h
  Author:     Bubli <kmachalkova@suse.cz>

*/


#ifndef NCPkgFilterRepo_h
#define NCPkgFilterRepo_h

#include <iosfwd>
#include <string>
#include <vector>
#include <algorithm>

#include <yui/ncurses/NCPadWidget.h>
#include <yui/ncurses/NCPopup.h>
#include <yui/ncurses/NCPushButton.h>
#include <yui/ncurses/NCTable.h>
#include <yui/ncurses/NCTablePad.h>

#include "NCZypp.h"


class NCTable;
class NCPushButton;
class NCPackageSelector;

class NCPkgRepoTag : public YTableCell
{

private:

    ZyppRepo repo;

public:
    /**
      * A helper class to hold a reference to zypp::Repository
      * for each repository table line
      * (actually it's a dummy column of the table)
      * @param repo zypp::Repository reference
      */

    NCPkgRepoTag ( ZyppRepo repo);

    // Nikdy, ale opravdu nikdy nenechavej v odvozene tride virtualni
    // destruktor, kdyz ani v puvodni neni, Bublino!
    ~NCPkgRepoTag() {};

    /*
     * Get repository reference from the line tag
     * @return ZyppRepo
     */

    ZyppRepo getRepo() const		{ return repo; }

};

class NCPkgRepoTable : public NCTable
{
private:

    NCPkgRepoTable & operator=( const NCPkgRepoTable & );
    NCPkgRepoTable            ( const NCPkgRepoTable & );

    NCPackageSelector *packager;
public:

    /**
      * A helper class to hold repository data in a neat table
      * widget
      * @param parent A parent widget
      * @param opt Widget options
      */

    NCPkgRepoTable  ( YWidget *parent, YTableHeader *tableHeader, NCPackageSelector *pkg);

    virtual ~NCPkgRepoTable() {};

    /**
      * Add one line (with tag) to the repositories table
      * @param ZyppRepo Reference to zypp::Repository
      * @param cols String std::vector with repository data (name + URL)
      */
    virtual void addLine( ZyppRepo r, const std::vector<std::string> & cols );

    /*
     * Fill header of repositories table (name + URL)
     */
    void fillHeader();

    /**
      * Get tag of repository table line on current index,
      * ( contains repository reference)
      * @param index Index of selected table line
      * @return NCPkgRepoTag* Tag of selected line
      */
    NCPkgRepoTag * getTag ( const int & index );

    /**
     * Get repository reference from selected line's tag
     * @param index Index of selected table line
     * @return ZyppRepo Associated zypp::Repository reference
     */
    ZyppRepo getRepo( int index );

    std::string showDescription (ZyppRepo r);

    virtual NCursesEvent wHandleInput ( wint_t ch );

    /**
      * Find single zypp::Product for this repository
      * (null product if multiple products found)
      * @param repo zypp::Repository
      * @return ZyppProduct
      */

    ZyppProduct findProductForRepo (ZyppRepo repo);

   /**
      * Add items to the repository list (assoc.
      * product name, if any, and URL)
      * @return bool (always true;-) )
      */
    bool fillRepoList();

    bool showRepoPackages();

};
#endif
