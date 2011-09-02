/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2011 Rallaz (rallazz@gmail.com)
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software 
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!  
**
**********************************************************************/

#ifndef QG_BLOCKWIDGET_H
#define QG_BLOCKWIDGET_H

#include <QWidget>
#include <QIcon>
#include <QAbstractTableModel>

#include "rs_blocklistlistener.h"

class QG_ActionHandler;
class QTableView;


/**
 * Implementation of a model to use in QG_BlockWidget
 */
class QG_BlockModel: public QAbstractTableModel {
public:
    enum {
        VISIBLE,
        NAME,
        LAST
    };
    QG_BlockModel(QObject * parent = 0);
    ~QG_BlockModel();
    Qt::ItemFlags flags ( const QModelIndex & /*index*/ ) {
            return Qt::ItemIsSelectable|Qt::ItemIsEnabled;}
    int columnCount(const QModelIndex &/*parent*/) const {return LAST;}
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QModelIndex parent ( const QModelIndex & index ) const;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    void setBlockList(RS_BlockList* bl);
    RS_Block *getBlock( int row );
    QModelIndex getIndex (RS_Block * blk);

private:
    QList<RS_Block*> listBlock;
    QIcon blockVisible;
    QIcon blockHidden;
};


/**
 * This is the Qt implementation of a widget which can view a 
 * block list.
 */
class QG_BlockWidget: public QWidget, public RS_BlockListListener {
    Q_OBJECT

public:
    QG_BlockWidget(QG_ActionHandler* ah, QWidget* parent,
                   const char* name=0, Qt::WFlags f = 0);
    ~QG_BlockWidget();

    void setBlockList(RS_BlockList* blockList) {
        this->blockList = blockList;
        update();
    }

    RS_BlockList* getBlockList() {
        return blockList;
    }

    void update();
    void activateBlock(RS_Block* block);

    virtual void blockAdded(RS_Block*) {
        update();
    }
    virtual void blockEdited(RS_Block*) {
        update();
    }
    virtual void blockRemoved(RS_Block*) {
        update();
    }
    virtual void blockToggled(RS_Block*) {
		update();
	}

signals:
	void escape();

public slots:
    void slotActivated(QModelIndex blockIdx);

protected:
    void contextMenuEvent(QContextMenuEvent *e);
	virtual void keyPressEvent(QKeyEvent* e);

private:
    RS_BlockList* blockList;
    QTableView* blockView;
    QG_BlockModel *blockModel;
    RS_Block* lastBlock;

    QG_ActionHandler* actionHandler;
};

#endif
