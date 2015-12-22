template<class T> class NoRGBIMonitorT;
typedef NoRGBIMonitorT<void> NoRGBIMonitor;

template<class T> class NoRGBIMonitorT : public Component
{
public:
    static String typeName() { return "NoRGBIMonitor"; }
    NoRGBIMonitorT(Component::Type type) : Component(type), _connector(this)
    {
        connector("", &_connector);
    }

    class Connector : public ::Connector
    {
    public:
        Connector(NoRGBIMonitor* c) : ::Connector(c) { }
        class Type : public NamedNullary<::Connector::Type, Type>
        {
        public:
            static String name() { return "NoRGBIMonitor.Connector"; }
            class Body : public NamedNullary<::Connector::Type, Type>::Body
            {
            public:
                bool compatible(::Connector::Type other) const
                {
                    return other == IBMCGA::RGBIConnector::Type();
                }
            };
        };
    protected:
        ::Connector::Type type() const { return Type(); }
        void connect(::Connector* other) { }
        Component::Type defaultComponentType(Simulator* simulator)
        {
            assert(false);
            return Component::Type();
        }
    };

    typedef Component::TypeHelper<NoRGBIMonitor> Type;
private:
    Connector _connector;
};

template<class T> class IBMCGAT : public ISA8BitComponent<IBMCGAT<T>>
{
public:
    static String typeName() { return "IBMCGA"; }
    IBMCGAT(Component::Type type)
      : ISA8BitComponent<IBMCGAT<T>>(type), _attr(0), _chrdata(0), _wait(0),
        _cycle(0), _bgri(0), _lightPenStrobe(false), _lightPenSwitch(true),
        _bgriSource(this), _ram(RAM::Type(simulator())), _rgbiConnector(this)
    {
        this->config("rom", &_rom);
        this->persist("memoryActive", &_memoryActive);
        this->persist("memoryAddress", &_memoryAddress, HexPersistenceType(4));
        this->persist("portActive", &_portActive);
        this->persist("portAddress", &_portAddress, HexPersistenceType(4));
        this->persist("mode", &_mode);
        this->persist("palette", &_palette);
        config("ram", &_ram, RAM::Type(simulator(), &_ram));
        connector("rgbiOutput", &_rgbiConnector);
    }
    void load(Value v)
    {
        ISA8BitComponent<IBMCGAT<T>>::load(v);
        String data = File(_rom, this->_simulator->directory()).contents();
        int length = 0x2000;
        _romdata.allocate(length);
        if (data.length() < length) {
            throw Exception(fileName + " is too short: " +
                decimal(data.length()) + " bytes found, " + decimal(length) +
                " bytes required");
        }
        for (int i = 0; i < length; ++i)
            _romdata[i] = data[i];
        _data = _ram.data();
    }
    void simulateCycle()
    {
        _cycle = (_cycle + 1) & 15;
        if (_cycle == 0) {
            _crtc.simulateCycle();

            int ma = (_crtc.memoryAddress() & 0x1fff) << 1;
            UInt8 ch = _data[ma];
            _attr = _data[ma + 1];
            _chrdata = _romdata[0x1800 + (ch << 3) + (_crtc.rowAddress() & 7)];
        }
        if ((_mode & 2) != 0) {
        }
        else {
            UInt8 tmp = _chrdata & (0x80 >> (_cycle >> 1));
            if(tmp) {
                _bgri = 0x0F;
            }
            else {
                _bgri = 0;
            }
            this->_bgriSource.produce(1);
        }
    }
    void setAddress(UInt32 address)
    {
        _memoryActive = ((address & 0x400f8000) == 0xb8000);
        _memoryAddress = address & 0x00003fff;
        _portActive = ((address & 0x400003f0) == 0x400003d0);
        _portAddress = address & 0x0000000f;
        this->_active = (_memoryActive || _portActive);
    }
    void read()
    {
        if (_memoryActive) {
            _wait = 8 + (16 - _cycle);
            this->set(_data[_memoryAddress]);
            return;
        }
        if (!_portActive)
            return;
        if ((_portAddress & 8) == 0) {
            this->set(_crtc.read((_portAddress & 1) != 0));
            return;
        }
        switch (_portAddress & 7) {
            case 2:
                this->set((_crtc.displayEnable() ? 0 : 1) |
                    (_lightPenStrobe ? 2 : 0) |
                    (_lightPenSwitch ? 4 : 0) |
                    (_crtc.verticalSync() ? 8 : 0) | 0xf0);
                break;
            case 3:
                _lightPenStrobe = false;
                this->set(0xff);
                break;
            case 4:
                activateLightPen();
                this->set(0xff);
                break;
            default:
                this->set(0xff);
                break;
        }
    }
    void write(UInt8 data)
    {
        if (_memoryActive) {
            _wait = 8 + (16 - _cycle);
            _data[_memoryAddress] = data;
        }
        if (!_portActive)
            return;
        if ((_portAddress & 8) == 0) {
            _crtc.write((_portAddress & 1) != 0, data);
            return;
        }
        switch (_portAddress & 7) {
            case 0:
                _mode = data;
                break;
            case 1:
                _palette = data;
                break;
            case 3:
                _lightPenStrobe = false;
                break;
            case 4:
                activateLightPen();
                break;
        }
    }
    UInt8 memory(UInt32 address)
    {
        if ((address & 0xf8000) == 0xb8000)
            return _data[address & 0x3fff];
        else
            return 0xff;
    }
    class BGRISource : public Source<BGRI>
    {
    public:
        BGRISource(IBMCGA* cga) : _cga(cga)
        {
        }
        void produce(int n)
        {
            Accessor<BGRI> acc = writer(n);
            acc.item() = this->_cga->_bgri |
                (this->_cga->_crtc.horizontalSync() ? 0x10 : 0) |
                (this->_cga->_crtc.verticalSync() ? 0x20 : 0);
            written(1);
        }
    private:
        IBMCGA* _cga;
    };
    class CompositeSource : public Source<UInt8> { void produce(int n) { } };
    BGRISource* bgriSource() { return &_bgriSource; }
    CompositeSource* compositeSource() { return &_compositeSource; }

    class RGBIConnector : public ::Connector
    {
        typedef ::Connector::Type CType;
    public:
        RGBIConnector(IBMCGA* cga) : ::Connector(cga), _cga(cga) { }
        void connect(::Connector* other)
        {
            // TODO
        }
        ::Connector::Type type() const { return Type(); }
        Component::Type defaultComponentType(Simulator* simulator)
        {
            return NoRGBIMonitor::Type(simulator);
        }

        class Type : public NamedNullary<CType, Type>
        {
        public:
            Type() { }
            Type(::Type type) : NamedNullary<CType, Type>(type) { }
            class Body : public NamedNullary<CType, Type>::Body
            {
            public:
                bool compatible(CType other) const
                {
                    return RGBIMonitorT<T>::Connector::Type(other).valid();
                }
            };
            static String name() { return "IBMCGA.RGBIConnector"; }
            bool valid() const { return body() != 0; }
            const Body* body() const { return as<Body>(); }
        };
    private:
        IBMCGA* _cga;
    };

private:
    void activateLightPen()
    {
        if (!_lightPenStrobe)
            _crtc.activateLightPen();
        _lightPenStrobe = true;
    }

    UInt8* _data;
    RGBIConnector _rgbiConnector;
    String _rom;
    Array<UInt8> _romdata;
    UInt8 _attr;
    UInt8 _chrdata;
    int _memoryAddress;
    bool _memoryActive;
    int _portAddress;
    bool _portActive;
    int _wait;
    int _cycle;
    UInt8 _mode;
    UInt8 _palette;
    UInt8 _bgri;
    Motorola6845CRTC _crtc;
    bool _lightPenStrobe;
    bool _lightPenSwitch;
    BGRISource _bgriSource;
    CompositeSource _compositeSource;
    RAM _ram;
};
