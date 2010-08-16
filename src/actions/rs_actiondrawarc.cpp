/****************************************************************************
**
** This file is part of the CADuntu project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (caduntu@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
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

#include "rs_actiondrawarc.h"

#include "rs_commands.h"
#include "rs_snapper.h"
#include "rs_dialogfactory.h"



RS_ActionDrawArc::RS_ActionDrawArc(RS_EntityContainer& container,
                                   RS_GraphicView& graphicView)
        :RS_PreviewActionInterface("Draw arcs",
                           container, graphicView) {

    reset();
}



RS_ActionDrawArc::~RS_ActionDrawArc() {}

QAction* RS_ActionDrawArc::createGUIAction(RS2::ActionType /*type*/, QObject* /*parent*/) {
/* RVT_PORT    QAction* action = new QAction(tr("Arc: Center, Point, Angles"),
                                  tr("&Center, Point, Angles"),
                                  QKeySequence(), NULL); */
    QAction* action = new QAction(tr("Arc: Center, Point, Angles"),NULL);
    action->setStatusTip(tr("Draw arcs"));
    return action;
}


void RS_ActionDrawArc::reset() {
    //bool rev = data.reversed;

    if (data.reversed) {
        data = RS_ArcData(RS_Vector(false),
                          0.0,
                          2*M_PI, 0.0,
                          true);
    } else {
        data = RS_ArcData(RS_Vector(false),
                          0.0,
                          0.0, 2*M_PI,
                          false);
    }
}



void RS_ActionDrawArc::init(int status) {
    RS_PreviewActionInterface::init(status);

    reset();
}



void RS_ActionDrawArc::trigger() {
    RS_PreviewActionInterface::trigger();

    RS_Arc* arc = new RS_Arc(container,
                             data);
    arc->setLayerToActive();
    arc->setPenToActive();
    container->addEntity(arc);

    // upd. undo list:
    if (document!=NULL) {
        document->startUndoCycle();
        document->addUndoable(arc);
        document->endUndoCycle();
    }

	graphicView->redraw(RS2::RedrawDrawing);
    graphicView->moveRelativeZero(arc->getCenter());

    setStatus(SetCenter);
    reset();

    RS_DEBUG->print("RS_ActionDrawArc::trigger(): arc added: %d",
                    arc->getId());
}



void RS_ActionDrawArc::mouseMoveEvent(RS_MouseEvent* e) {
    RS_DEBUG->print("RS_ActionDrawArc::mouseMoveEvent begin");

    RS_Vector mouse = snapPoint(e);
    switch (getStatus()) {
    case SetCenter:
        data.center = mouse;
        break;

    case SetRadius:
        if (data.center.valid) {
            data.radius = data.center.distanceTo(mouse);
            deletePreview();
            preview->addEntity(new RS_Circle(preview,
                                             RS_CircleData(data.center,
                                                           data.radius)));
            drawPreview();
        }
        break;

    case SetAngle1:
        data.angle1 = data.center.angleTo(mouse);
        if (data.reversed) {
            data.angle2 = RS_Math::correctAngle(data.angle1-M_PI/3);
        } else {
            data.angle2 = RS_Math::correctAngle(data.angle1+M_PI/3);
        }
        deletePreview();
        preview->addEntity(new RS_Arc(preview,
                                      data));
        drawPreview();
        break;

    case SetAngle2:
        data.angle2 = data.center.angleTo(mouse);
        deletePreview();
        preview->addEntity(new RS_Arc(preview,
                                      data));
        drawPreview();
        break;

    case SetIncAngle:
        data.angle2 = data.angle1 + data.center.angleTo(mouse);
        deletePreview();
        preview->addEntity(new RS_Arc(preview,
                                      data));
        drawPreview();
        break;

    case SetChordLength: {
            double x = data.center.distanceTo(mouse);
            if (fabs(x/(2*data.radius))<=1.0) {
                data.angle2 = data.angle1 + asin(x/(2*data.radius)) * 2;
                deletePreview();
                preview->addEntity(new RS_Arc(preview,
                                              data));
                drawPreview();
            } 
        }
        break;

    default:
        break;

    }

    RS_DEBUG->print("RS_ActionDrawArc::mouseMoveEvent end");
}



void RS_ActionDrawArc::mouseReleaseEvent(RS_MouseEvent* e) {
    if (RS2::qtToRsButtonState(e->button())==RS2::LeftButton) {
        RS_CoordinateEvent ce(snapPoint(e));
        coordinateEvent(&ce);
    } else if (RS2::qtToRsButtonState(e->button())==RS2::RightButton) {
        deletePreview();
        init(getStatus()-1);
    }
}



void RS_ActionDrawArc::coordinateEvent(RS_CoordinateEvent* e) {
    if (e==NULL) {
        return;
    }
    RS_Vector mouse = e->getCoordinate();

    switch (getStatus()) {
    case SetCenter:
        data.center = mouse;
        graphicView->moveRelativeZero(mouse);
        setStatus(SetRadius);
        break;

    case SetRadius:
        if (data.center.valid) {
            data.radius = data.center.distanceTo(mouse);
        }
        setStatus(SetAngle1);
        break;

    case SetAngle1:
        data.angle1 = data.center.angleTo(mouse);
        setStatus(SetAngle2);
        break;

    case SetAngle2:
        data.angle2 = data.center.angleTo(mouse);
        trigger();
        break;

    case SetIncAngle:
        data.angle2 = data.angle1 + data.center.angleTo(mouse);
        trigger();
        break;

    case SetChordLength: {
            double x = data.center.distanceTo(mouse);
            if (fabs(x/(2*data.radius))<=1.0) {
                data.angle2 = data.angle1 + asin(x/(2*data.radius)) * 2;
                trigger();
            }
        }
        break;

    default:
        break;
    }
}



void RS_ActionDrawArc::commandEvent(RS_CommandEvent* e) {
    RS_String c = e->getCommand().lower();

    if (RS_COMMANDS->checkCommand("help", c)) {
        if (RS_DIALOGFACTORY!=NULL) {
            RS_DIALOGFACTORY->commandMessage(msgAvailableCommands()
                                             + getAvailableCommands().join(", "));
        }
        return;
    }

    if (RS_COMMANDS->checkCommand("reversed", c)) {
        e->accept();
        setReversed(!isReversed());

        if (RS_DIALOGFACTORY!=NULL) {
            RS_DIALOGFACTORY->requestOptions(this, true, true);
        }
        return;
    }

    switch (getStatus()) {

    case SetRadius: {
            bool ok;
            double r = RS_Math::eval(c, &ok);
            if (ok==true) {
                data.radius = r;
                setStatus(SetAngle1);
            } else {
                if (RS_DIALOGFACTORY!=NULL) {
                    RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
                }
            }
        }
        break;

    case SetAngle1: {
            bool ok;
            double a = RS_Math::eval(c, &ok);
            if (ok==true) {
                data.angle1 = RS_Math::deg2rad(a);
                setStatus(SetAngle2);
            } else {
                if (RS_DIALOGFACTORY!=NULL) {
                    RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
                }
            }
        }
        break;

    case SetAngle2: {
            if (RS_COMMANDS->checkCommand("angle", c)) {
                setStatus(SetIncAngle);
            } else if (RS_COMMANDS->checkCommand("chord length", c)) {
                setStatus(SetChordLength);
            } else {
                bool ok;
                double a = RS_Math::eval(c, &ok);
                if (ok==true) {
                    data.angle2 = RS_Math::deg2rad(a);
                    trigger();
                } else {
                    if (RS_DIALOGFACTORY!=NULL) {
                        RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
                    }
                }
            }
        }
        break;

    case SetIncAngle: {
            bool ok;
            double a = RS_Math::eval(c, &ok);
            if (ok==true) {
                data.angle2 = data.angle1 + RS_Math::deg2rad(a);
                trigger();
            } else {
                if (RS_DIALOGFACTORY!=NULL) {
                    RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
                }
            }
        }
        break;

    case SetChordLength: {
            bool ok;
            double l = RS_Math::eval(c, &ok);
            if (ok==true) {
                if (fabs(l/(2*data.radius))<=1.0) {
                    data.angle2 = data.angle1 + asin(l/(2*data.radius)) * 2;
                    trigger();
                } else {
                    if (RS_DIALOGFACTORY!=NULL) {
                        RS_DIALOGFACTORY->commandMessage(
                            tr("Not a valid chord length"));
                    }
                }
            } else {
                if (RS_DIALOGFACTORY!=NULL) {
                    RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
                }
            }
        }
        break;

    default:
        break;
    }
}



RS_StringList RS_ActionDrawArc::getAvailableCommands() {
    RS_StringList cmd;
    cmd += RS_COMMANDS->command("reversed");
    return cmd;
}


void RS_ActionDrawArc::updateMouseButtonHints() {
    if (RS_DIALOGFACTORY!=NULL) {
        switch (getStatus()) {
        case SetCenter:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify center"), tr("Cancel"));
            break;
        case SetRadius:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify radius"), tr("Back"));
            break;
        case SetAngle1:
            RS_DIALOGFACTORY->updateMouseWidget(
                tr("Specify start angle:"), tr("Back"));
            break;
        case SetAngle2:
            RS_DIALOGFACTORY->updateMouseWidget(
                tr("Specify end angle or [Angle/chord Length]"),
                tr("Back"));
            break;
        case SetIncAngle:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify included angle:"),
                                                tr("Back"));
            break;
        case SetChordLength:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify chord length:"),
                                                tr("Back"));
            break;
        default:
            RS_DIALOGFACTORY->updateMouseWidget("", "");
            break;
        }
    }
}



void RS_ActionDrawArc::showOptions() {
    RS_ActionInterface::showOptions();

    if (RS_DIALOGFACTORY!=NULL) {
        RS_DIALOGFACTORY->requestOptions(this, true);
    }
}



void RS_ActionDrawArc::hideOptions() {
    RS_ActionInterface::hideOptions();

    if (RS_DIALOGFACTORY!=NULL) {
        RS_DIALOGFACTORY->requestOptions(this, false);
    }
}



void RS_ActionDrawArc::updateMouseCursor() {
    graphicView->setMouseCursor(RS2::CadCursor);
}



void RS_ActionDrawArc::updateToolBar() {
    if (RS_DIALOGFACTORY!=NULL) {
        if (!isFinished()) {
            RS_DIALOGFACTORY->requestToolBar(RS2::ToolBarSnap);
        } else {
            RS_DIALOGFACTORY->requestToolBar(RS2::ToolBarArcs);
        }
    }
}


// EOF

