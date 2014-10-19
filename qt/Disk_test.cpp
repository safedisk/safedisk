#include "Disk.h"

#include <QtTest>

class TestDisk: public QObject
{
	Q_OBJECT

public:
	TestDisk()
	{
	}

private Q_SLOTS:
	void testCase1()
	{
		QVERIFY2(true, "Failure");
	}
};

QTEST_APPLESS_MAIN(TestDisk)

#include "Disk_test.moc"
