/* vi: set sw=4 ts=4:
 *
 * Copyright (C) 2001 - 2010 Christian Hohnstaedt.
 *
 * All rights reserved.
 */

#include "db.h"
#include "exception.h"
#include <stdlib.h>
#include <QtCore/QByteArray>

QByteArray filename2bytearray(const QString &fname)
{
#ifdef WIN32
	return fname.toLocal8Bit();
#else
	return fname.toUtf8();
#endif
}

QString filename2QString(const char *fname)
{
#ifdef WIN32
	return QString::fromLocal8Bit(fname);
#else
	return QString::fromUtf8(fname);
#endif
}

static QByteArray fileNameEncoderFunc(const QString &fileName)
{
	return filename2bytearray(fileName);
}

static QString fileNAmeDecoderFunc(const QByteArray &localFileName)
{
	return filename2QString(localFileName.constData());
}


int main(int argc, char *argv[])
{
	if (argc < 2)
		return 1;

	QFile::setEncodingFunction(fileNameEncoderFunc);
	QFile::setDecodingFunction(fileNAmeDecoderFunc);

	QString database = filename2QString(argv[1]);

	db mydb(database);
	unsigned char *p;
	db_header_t h;
	int i=0;
	size_t last_end = 0;
	QList<size_t> errs;
	int format=16;
	const char *type[] = {
		"(none)", "Software Key", "Request", "Certificate",
		"Revocation", "Template", "Setting", "Token key"
	};
#define FW_IDX 5
#define FW_TYPE -13
#define FW_VER 3
#define FW_SIZE 6
#define FW_FLAGS 6
	try {
		mydb.first(0);
		//QString fmt = QString("%1 %2 %3 %4 %5 %6 %7");
		QString fmt = QString("%1 | %2 | %3 | %4 | %5 | %6 | %7");
		puts(CCHAR(fmt  .arg("Index", FW_IDX)
				.arg("Type", FW_TYPE)
				.arg("Ver", FW_VER)
				.arg("Offset", FW_SIZE)
				.arg("Length", FW_SIZE)
				.arg("End", FW_SIZE)
				.arg("Name")));
		while (!mydb.eof()) {
			p = mydb.load(&h);
			free(p);
			if (last_end != (size_t)mydb.head_offset)
				errs << mydb.head_offset;
			last_end = mydb.head_offset + h.len;
			if (h.type > smartCard)
				h.type = 0;
			puts(CCHAR(fmt  .arg(i++, FW_IDX)
					.arg(type[h.type], FW_TYPE)
					.arg(h.version, FW_VER)
					.arg(mydb.head_offset, FW_SIZE, format)
					.arg(h.len, FW_SIZE, format)
					.arg(last_end -1, FW_SIZE, format)
					.arg(h.name)));
			if (mydb.next(0))
				break;
		}
		if (errs.size() > 0) {
			fputs("Garbage found at offset:", stdout);
			foreach(size_t e, errs)
				puts(CCHAR(QString(" %1").arg(e)));
			puts("");
		}
	} catch (errorEx &ex) {
		printf("Exception: '%s'\n", ex.getCString());
		return 1;
	}
	return 0;
}