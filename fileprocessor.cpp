#include "fileprocessor.h"
#include <QString>

FileProcessor::FileProcessor(QObject* parent) : QObject(parent) {}
FileProcessor::~FileProcessor() {}
QString FileProcessor::readTxtFile(const QString& path) { return QString(); }
QString FileProcessor::readComFile(const QString& path) { return QString(); }
bool FileProcessor::saveTxtFile(const QString& path, const QString& content) { return false; }
