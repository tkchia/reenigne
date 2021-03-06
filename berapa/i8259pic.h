template<class T> class Intel8259PICT
  : public ISA8BitComponentBase<Intel8259PICT<T>>
{
    using Base = ISA8BitComponentBase<Intel8259PICT<T>>;
public:
    static String typeName() { return "Intel8259PIC"; }
    Intel8259PICT(Component::Type type)
      : Base(type), _interruptReady(false), _secondAck(false),
        _state(stateReady), _imr(0xff),
        _irqConnector{this, this, this, this, this, this, this, this},
        _intConnector(this)
    {
        this->persist("address", &_address);
        this->persist("offset", &_offset);
        this->persist("irr", &_irr);
        this->persist("imr", &_imr);
        this->persist("isr", &_isr);
        this->persist("icw1", &_icw1);
        this->persist("icw4", &_icw4);
        this->persist("interrupt", &_interrupt);
        this->persist("interruptReady", &_interruptReady);
        this->persist("secondAck", &_secondAck);
        this->persist("interruptNumber", &_interruptNumber);
        for (int i = 0; i < 8; ++i) {
            _irqConnector[i].init(i);
            this->connector("irq" + decimal(i), &_irqConnector[i]);
        }
        this->connector("int", &_intConnector);
    }
    class Connector : public InputConnector<bool>
    {
    public:
        Connector(Intel8259PIC* pic) : InputConnector(pic) { }
        void init(int i) { _i = i; }
        void setData(Tick t, bool v)
        {
            static_cast<Intel8259PIC*>(component())->setIRQ(t, _i, v);
        }
        int _i;
    };
    void setIRQ(Tick t, int i, bool v)
    {
        // TODO
    }


    ISA8BitComponent* setAddressReadIO(Tick tick, UInt32 address)
    {
        _address = address & 1;
        return this;
    }
    ISA8BitComponent* setAddressWriteIO(Tick tick, UInt32 address)
    {
        _address = address & 1;
        return this;
    }
    UInt8 read(Tick tick)
    {
        if (_address == 1)
            return _imr;
        // TODO
        return 0xff;
    }
    UInt8 readAcknowledgeByte(Tick tick)
    {
        this->runTo(tick);
        if (_secondAck) {
            _secondAck = false;
            return _interruptNumber;
        }
        _secondAck = true;
        _interruptReady = false;
        return 0xff;
    }
    void write(Tick tick, UInt8 data)
    {
        if (_address == 0) {
            if ((data & 0x10) != 0) {
                _icw1 = data & 0x0f;
                _state = stateICW2;
            }
            return;
        }
        switch (_state) {
            case stateICW2:
                _offset = data;
                _state = stateICW3;
                break;
            case stateICW3:
                if ((_icw1 & 1) != 0)
                    _state = stateICW4;
                else
                    _state = stateReady;
                break;
            case stateICW4:
                _icw4 = data;
                _state = stateReady;
                break;
            case stateReady:
                _imr = data;
                break;
        }
    }
    bool interruptRequest() { return _interruptReady; }

    void requestInterrupt(int line)
    {
        if (_state == stateReady) {
            _interruptNumber = line + _offset;
            _interruptReady = (((~_imr) & (1 << line)) != 0);
            _interrupt = false;
            _secondAck = false;
        }
        else
            _interrupt = false;
    }
    void setBus(ISA8BitBus* bus)
    {
        Base::setBus(bus);
        this->_bus->setPIC(this);
    }
private:
    UInt8 _interruptNumber;
    enum State
    {
        stateReady,
        stateICW2,
        stateICW3,
        stateICW4,
    } _state;

    bool _interrupt;
    bool _interruptReady;
    bool _secondAck;

    int _address;
    UInt8 _offset;
    UInt8 _irr;
    UInt8 _imr;
    UInt8 _isr;

    UInt8 _icw1;
    UInt8 _icw4;

    Connector _irqConnector[8];
    OutputConnector<bool> _intConnector;
};