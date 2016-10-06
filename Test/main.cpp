#include "gtest/gtest.h"

#include <QCoreApplication>
#include <QTimer>

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);

	QCoreApplication app(argc, argv);
	int ret =  RUN_ALL_TESTS();
	return ret;
}