#ifndef __iodevice_h__
#define __iodevice_h__

#include "types.h"
#include <QList>
#include <QHash>

namespace Vomit
{

class IODevice
{
public:
	IODevice(const char *name);
	virtual ~IODevice();

	const char *name() const;

	virtual BYTE in8();
	virtual void out8(BYTE);

	static QList<IODevice*> & devices();
	static QHash<WORD, IODevice*>& readDevices();
	static QHash<WORD, IODevice*>& writeDevices();

	QList<WORD> ports() const;

protected:
	enum ListenMask {
		Read = 1,
		Write = 2,
		ReadWrite = 3
	};
	virtual void listen(WORD port, ListenMask mask);

private:
	const char *m_name;
	QList<WORD> m_ports;
};

}

#endif