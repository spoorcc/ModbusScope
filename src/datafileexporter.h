#ifndef DATAFILEEXPORTER_H
#define DATAFILEEXPORTER_H

#include <QObject>
#include <QStringList>

/* Forward declaration */
class SettingsModel;
class GuiModel;
class ExtendedGraphView;

class DataFileExporter : public QObject
{
    Q_OBJECT
public:
    explicit DataFileExporter(GuiModel *pGuiModel, SettingsModel *pSettingsModel, ExtendedGraphView * pGraphView, QObject *parent = 0);
    ~DataFileExporter();

    void enableExporterDuringLog();
    void disableExporterDuringLog();

    void exportDataFile(QString dataFile);

signals:

public slots:
    void exportDataLine(double timeData, QList <double> dataValues);

private:
    void flushExportBuffer();
    void exportDataHeader();
    QString constructDataHeader(bool bDuringLog);
    QString createLabelRow();
    QString formatData(double timeData, QList<double> dataValues);
    void writeToFile(QString filePath, QStringList logData);
    void clearFile(QString filePath);

    GuiModel * _pGuiModel;
    SettingsModel * _pSettingsModel;
    ExtendedGraphView * _pGraphView;

    QStringList _dataExportBuffer;
    quint64 lastLogTime;

    static const quint64 _cLogBufferTimeout = 1000; /* in milliseconds */

};

#endif // DATAFILEEXPORTER_H
