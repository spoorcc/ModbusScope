
#include "graphviewzoom.h"

GraphViewZoom::GraphViewZoom(GuiModel* pGuiModel, MyQCustomPlot* pPlot, QObject *parent) :
    QObject(parent)
{
    _pRubberBand = nullptr;
    _pGuiModel = pGuiModel;
    _pGraphview = dynamic_cast<BasicGraphView*>(parent);
    _pPlot = pPlot;

    connect(_pGuiModel, SIGNAL(zoomStateChanged()), this, SLOT(handleZoomStateChanged()));
}

GraphViewZoom::~GraphViewZoom()
{

}

void GraphViewZoom::handleZoomStateChanged()
{
    if (_pGuiModel->zoomState() != GuiModel::ZOOM_IDLE)
    {
        _pPlot->setCursor(Qt::CrossCursor);
    }
    else
    {
        _pPlot->setCursor(Qt::ArrowCursor);
    }
}

/*!
 * \brief GraphViewZoom::handleMousePress
 * \param event
 * \return true when event already handled
 */
bool GraphViewZoom::handleMousePress(QMouseEvent *event)
{
    if (_pGuiModel->zoomState() == GuiModel::ZOOM_TRIGGERED)
    {
        /* Disable range drag when zooming is active */
        _pPlot->setInteraction(QCP::iRangeDrag, false);
        _pPlot->setInteraction(QCP::iRangeZoom, false);

        _pGuiModel->setZoomState(GuiModel::ZOOM_SELECTING);

        _selectionOrigin = event->pos();
        if (!_pRubberBand)
        {
            _pRubberBand = new QRubberBand(QRubberBand::Rectangle, _pPlot);
        }
        _pRubberBand->setGeometry(QRect(_selectionOrigin, QSize()));
        _pRubberBand->show();

        return true;
    }

    return false;
}

bool GraphViewZoom::handleMouseWheel()
{
    if (_pGuiModel->zoomState() != GuiModel::ZOOM_IDLE)
    {
        /* Zoom has precedence when not idle */
        return true;
    }

    return false;

}

bool GraphViewZoom::handleMouseRelease()
{
    if (_pGuiModel->zoomState() == GuiModel::ZOOM_SELECTING)
    {
        _pRubberBand->hide();

        _pGuiModel->setyAxisScale(BasicGraphView::SCALE_MANUAL); // change to manual scaling
        _pGuiModel->setxAxisScale(BasicGraphView::SCALE_MANUAL);

        /* Perform zoom based on selected rubberband */
        performZoom();

        /* Reset state */
        _pGuiModel->setZoomState(GuiModel::ZOOM_IDLE);

        return true;
    }

    return false;
}


bool GraphViewZoom::handleMouseMove(QMouseEvent *event)
{
    if (_pGuiModel->zoomState() == GuiModel::ZOOM_SELECTING)
    {
        /* Draw rectangle */
        _pRubberBand->setGeometry(QRect(_selectionOrigin, event->pos()).normalized());

        return true;
    }

    return false;
}

void GraphViewZoom::performZoom(void)
{
    const double correctTopLeftX = _pGraphview->pixelToClosestKey(_pRubberBand->geometry().topLeft().x());
    const double correctTopLeftY = _pGraphview->pixelToClosestValue(_pRubberBand->geometry().topLeft().y());

    const double correctBottomRightX = _pGraphview->pixelToClosestKey(_pRubberBand->geometry().bottomRight().x());
    const double correctBottomRightY = _pGraphview->pixelToClosestValue(_pRubberBand->geometry().bottomRight().y());

    _pPlot->xAxis->setRange(correctTopLeftX, correctBottomRightX);
    _pPlot->yAxis->setRange(correctBottomRightY, correctTopLeftY);
}

