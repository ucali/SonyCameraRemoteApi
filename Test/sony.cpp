#include "sonycamera.h"

#include "gtest/gtest.h"

#include <QEventLoop>
#include <future>

std::unique_ptr<QEventLoop> loop;
std::unique_ptr<Sony::Camera> cam;

class ScopedLoop {
public:
	ScopedLoop(QEventLoop& l) : loop(l) {  }
	~ScopedLoop() { loop.quit(); }

	QEventLoop& loop;
};

/*TEST(SONY_REMOTE_API, DISCOVER_SYNC) {
	QEventLoop loop;
	cam.reset(new Sony::Camera);

	cam->DiscoverSync(5, [&loop](bool ok) {
		ScopedLoop l(loop);
		ASSERT_TRUE(ok);
	});

	loop.exec();
}*/

TEST(SONY_REMOTE_API, DISCOVER_ASYNC) {

	loop.reset(new QEventLoop);
	cam.reset(new Sony::Camera);
	cam->DiscoverAsync(5, [](bool ok) {
		ScopedLoop l(*loop);
		ASSERT_TRUE(ok);
	});

	loop->exec();
}

TEST(SONY_REMOTE_API, COMMAND_FAILURE) {
	Sony::Command::Ptr cmd(new Sony::Command("testCommand"));
	QObject::connect(cmd.get(), &Sony::Command::onResponse, [] (Sony::Response r) {
		ScopedLoop l(*loop);
		ASSERT_FALSE(r.Error.code == 0);
	});
	cmd->SendTo(QUrl("http://127.0.0.1"));
	loop->exec();
}

TEST(SONY_REMOTE_API, ZOOM_TELE) {
	ASSERT_TRUE(cam->IsInitialized());

	cam->Zoomer()->Tele([](bool ok) {
		ScopedLoop l(*loop);
		ASSERT_TRUE(ok);
		
		QThread::sleep(3);
	});

	loop->exec();
}

TEST(SONY_REMOTE_API, ZOOM_WIDE) {
	ASSERT_TRUE(cam->IsInitialized());

	cam->Zoomer()->Wide([](bool ok) {
		ScopedLoop l(*loop);
		ASSERT_TRUE(ok);

		QThread::sleep(3);
	});

	loop->exec();
}

TEST(SONY_REMOTE_API, GRAB_IMAGE) {
	ASSERT_TRUE(cam->IsInitialized());

	Sony::GrabberService::ImageCallback c = [](QImage im) {
		ScopedLoop l(*loop);
		ASSERT_TRUE(!im.isNull());

		ASSERT_TRUE(im.save("grabbed_image.jpg", "JPEG", 100));
	};

	cam->Grabber()->Take(c);
	loop->exec();
}

TEST(SONY_REMOTE_API, GRAB_BYTES) {
	ASSERT_TRUE(cam->IsInitialized());

	Sony::GrabberService::BytesCallback c = [](QByteArray im) {
		ScopedLoop l(*loop);

		ASSERT_TRUE(im.size());

		QFile f("grabbed_bytes.jpeg");
		f.open(QFile::WriteOnly);
		f.write(im);
		f.close();
	};

	cam->Grabber()->Take(c);
	loop->exec();
}

TEST(SONY_REMOTE_API, GRAB_TO_FILE) {
	ASSERT_TRUE(cam->IsInitialized());


	Sony::GrabberService::ResultCallback c = [](bool ok) {
		ScopedLoop l(*loop);
		ASSERT_TRUE(ok);
		ASSERT_TRUE(QFile::exists("grabbed_file.jpg"));
	};

	cam->Grabber()->Take("grabbed_file.jpg", c);
	loop->exec();
}

class SonyTest : public ::testing::Test {
public:
	void Exec(const std::function<void(void)>& f) {

		cam->DiscoverAsync(5, [this, f](bool ok) {
			if (!ok || cam->IsInitialized() == false) {
				Quit();
				FAIL();
			} else {
				f();
			}
		});
		loop.exec();
	}

	void Quit() {
		loop.quit();
	}

protected:
	virtual void SetUp() {
		cam.reset(new Sony::Camera);
	}

	virtual void TearDown() {
		loop.quit();
		cam.reset();

	}

	QEventLoop loop;
	std::unique_ptr<Sony::Camera> cam;
};

TEST_F(SonyTest, FIXTURE_LIVE_VIEW) {
	int i = 0;
	Exec([this, &i] {
		cam->Live()->Start([this, &i](QImage im) {
			i++;

			EXPECT_TRUE(!im.isNull());
			EXPECT_TRUE(im.save("frame_" + QString::number(i) + ".jpg"));
			if (i > 3) {
				cam->Live()->Stop();
				Quit();
			}
		});
	});
}

TEST_F(SonyTest, FIXTURE_EXPOSURE) {
	int i = 0;
	Exec([this, &i] {
		cam->Exposure()->SetExposureMode("Program Auto", [this](bool ok) {
			EXPECT_TRUE(ok);
			Quit();
		});
	});
}
