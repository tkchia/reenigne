#include "alfe/main.h"
#include "alfe/complex.h"
#include "alfe/terminal6.h"
#include "alfe/bitmap_png.h"
#include "alfe/evaluate.h"
#include "alfe/ntsc_decode.h"

template<class T> class CaptureBitmapWindowTemplate;
typedef CaptureBitmapWindowTemplate<void> CaptureBitmapWindow;

template<class T> class DecoderThreadTemplate : public Thread
{
public:
    DecoderThreadTemplate() : _ending(false) { }
    void setWindow(CaptureBitmapWindow* window) { _window = window; }
    void go() { _go.signal(); }
    void end() { _ending = true; go(); }
private:
    void threadProc()
    {
        while (!_ending) {
            _go.wait();
            if (_ending)
                break;
            _window->update();
        }
    }

    Event _go;
    bool _ending;
    CaptureBitmapWindow* _window;
};

typedef DecoderThreadTemplate<void> DecoderThread;

class CaptureWindow;

template<class T> class CaptureBitmapWindowTemplate : public BitmapWindow
{
public:
    ~CaptureBitmapWindowTemplate()
    {
        _thread.end();
    }
    void setCaptureWindow(CaptureWindow* captureWindow)
    {
        _captureWindow = captureWindow;
    }
    void create()
    {
        setSize(Vector(960, 720));

        _vbiCapPipe = File("\\\\.\\pipe\\vbicap", true).openPipe();
        _vbiCapPipe.write<int>(1);

        int samples = 450*1024;
        int sampleSpaceBefore = 256;
        int sampleSpaceAfter = 256;
        _buffer.allocate(sampleSpaceBefore + samples + sampleSpaceAfter);
        _b = &_buffer[0] + sampleSpaceBefore;
        for (int i = 0; i < sampleSpaceBefore; ++i)
            _b[i - sampleSpaceBefore] = 0;
        for (int i = 0; i < sampleSpaceAfter; ++i)
            _b[i + samples] = 0;

        _decoder.setInputBuffer(_b);
        _decoder.setOutputPixelsPerLine(1140);
        _decoder.setYScale(3);

        BitmapWindow::create();
        _thread.setWindow(this);
        _thread.start();
    }

    void update()
    {
        _vbiCapPipe.read(_b, 1024*450);
        _decoder.setOutputBuffer(_bitmap);
        _decoder.decode();
        Bitmap<DWORD> nextBitmap = setNextBitmap(_bitmap);
        invalidate();
        _bitmap = nextBitmap;
    }

    void draw()
    {
        if (!_bitmap.valid())
            _bitmap = Bitmap<DWORD>(Vector(960, 720));
        _thread.go();
    }

    void paint()
    {
        _captureWindow->restart();
    }

    void setBrightness(double brightness)
    {
        _decoder.setBrightness(brightness);
    }
    void setSaturation(double saturation)
    {
        _decoder.setSaturation(saturation);
    }
    void setContrast(double contrast) { _decoder.setContrast(contrast); }
    void setHue(double hue) { _decoder.setHue(hue); }

private:
    Bitmap<DWORD> _bitmap;

    CaptureWindow* _captureWindow;

    NTSCCaptureDecoder<DWORD> _decoder;

    AutoHandle _vbiCapPipe;

    Array<Byte> _buffer;
    Byte* _b;

    DecoderThread _thread;
};

template<class T> class BrightnessSliderWindowTemplate : public Slider
{
public:
    void setHost(CaptureWindow* host) { _host = host; }
    void valueSet(double value) { _host->setBrightness(value); }
    void create()
    {
        setRange(-255, 255);
        Slider::create();
    }
private:
    CaptureWindow* _host;
};
typedef BrightnessSliderWindowTemplate<void> BrightnessSliderWindow;

template<class T> class SaturationSliderWindowTemplate : public Slider
{
public:
    void setHost(CaptureWindow* host) { _host = host; }
    void valueSet(double value) { _host->setSaturation(value); }
    void create()
    {
        setRange(0, 2);
        Slider::create();
    }
private:
    CaptureWindow* _host;
};
typedef SaturationSliderWindowTemplate<void> SaturationSliderWindow;

template<class T> class ContrastSliderWindowTemplate : public Slider
{
public:
    void setHost(CaptureWindow* host) { _host = host; }
    void valueSet(double value) { _host->setContrast(value); }
    void create()
    {
        setRange(0, 4);
        Slider::create();
    }
private:
    CaptureWindow* _host;
};
typedef ContrastSliderWindowTemplate<void> ContrastSliderWindow;

template<class T> class HueSliderWindowTemplate : public Slider
{
public:
    void setHost(CaptureWindow* host) { _host = host; }
    void valueSet(double value) { _host->setHue(value); }
    void create()
    {
        setRange(-180, 180);
        Slider::create();
    }
private:
    CaptureWindow* _host;
};
typedef HueSliderWindowTemplate<void> HueSliderWindow;

class CaptureWindow : public RootWindow
{
public:
    void restart() { _animated.restart(); }
    void setWindows(Windows* windows)
    {
        _output.setCaptureWindow(this);

        add(&_output);
        add(&_animated);

        add(&_brightnessCaption);
        add(&_brightness);
        add(&_brightnessText);
        add(&_saturationCaption);
        add(&_saturation);
        add(&_saturationText);
        add(&_contrastCaption);
        add(&_contrast);
        add(&_contrastText);
        add(&_hueCaption);
        add(&_hue);
        add(&_hueText);

        _animated.setDrawWindow(&_output);
        _animated.setRate(60);
        RootWindow::setWindows(windows);
    }
    void create()
    {
        _brightnessCaption.setText("Brightness: ");
        _saturationCaption.setText("Saturation: ");
        _contrastCaption.setText("Contrast: ");
        _hueCaption.setText("Hue: ");

        _brightness.setHost(this);
        _saturation.setHost(this);
        _contrast.setHost(this);
        _hue.setHost(this);

        setText("NTSC capture and decode");
        setSize(Vector(1321, 760 + 23));
        RootWindow::create();
        _animated.start();

        _brightness.setValue(-26);
        _contrast.setValue(1.65);
        _saturation.setValue(0.30);
        _hue.setValue(0);
    }
    void sizeSet(Vector size)
    {
        _output.setPosition(Vector(20, 20));
        int w = _output.right() + 20;

        Vector vSpace(0, 15);

        _brightness.setSize(Vector(301, 24));
        _brightness.setPosition(Vector(w, 20));
        _brightnessCaption.setPosition(_brightness.bottomLeft() + vSpace);
        _brightnessText.setPosition(_brightnessCaption.topRight());

        _saturation.setSize(Vector(301, 24));
        _saturation.setPosition(_brightnessCaption.bottomLeft() + 2*vSpace);
        _saturationCaption.setPosition(_saturation.bottomLeft() + vSpace);
        _saturationText.setPosition(_saturationCaption.topRight());

        _contrast.setSize(Vector(301, 24));
        _contrast.setPosition(_saturationCaption.bottomLeft() + 2*vSpace);
        _contrastCaption.setPosition(_contrast.bottomLeft() + vSpace);
        _contrastText.setPosition(_contrastCaption.topRight());

        _hue.setSize(Vector(301, 24));
        _hue.setPosition(_contrastCaption.bottomLeft() + 2*vSpace);
        _hueCaption.setPosition(_hue.bottomLeft() + vSpace);
        _hueText.setPosition(_hueCaption.topRight());

        RootWindow::sizeSet(size);
    }
    void setBrightness(double brightness)
    {
        _output.setBrightness(brightness);
        _brightnessText.setText(format("%f", brightness));
        _brightnessText.size();
    }
    void setSaturation(double saturation)
    {
        _output.setSaturation(saturation);
        _saturationText.setText(format("%f", saturation));
        _saturationText.size();
    }
    void setContrast(double contrast)
    {
        _output.setContrast(contrast);
        _contrastText.setText(format("%f", contrast));
        _contrastText.size();
    }
    void setHue(double hue)
    {
        _output.setHue(hue);
        _hueText.setText(format("%f", hue));
        _hueText.size();
    }

    void keyboardCharacter(int character)
    {
        if (character == VK_ESCAPE)
            remove();
    }
private:
    CaptureBitmapWindow _output;
    AnimatedWindow _animated;
    TextWindow _brightnessCaption;
    BrightnessSliderWindow _brightness;
    TextWindow _brightnessText;
    TextWindow _saturationCaption;
    SaturationSliderWindow _saturation;
    TextWindow _saturationText;
    TextWindow _contrastCaption;
    ContrastSliderWindow _contrast;
    TextWindow _contrastText;
    TextWindow _hueCaption;
    HueSliderWindow _hue;
    TextWindow _hueText;

};

class Program : public WindowProgram<CaptureWindow> { };