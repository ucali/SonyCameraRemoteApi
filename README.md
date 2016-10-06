# SONY CAMERA REMOTE API - Qt5 WIFI driver #

QX10 Control library.

```C++

Sony::Camera cam;
cam.DiscoverAsync(5, [] (bool ok) {
    ASSERT_TRUE(ok);
});

...

Sony::GrabberService::ImageCallback c = [] (QImage im) {
    ASSERT_TRUE(!im.isNull());
};
cam.Grabber()->Take(c);

...

cam.Live()->Start([] (QImage im) {
    EXPECT_TRUE(!im.isNull());
});
cam.Live()->Stop();

```

