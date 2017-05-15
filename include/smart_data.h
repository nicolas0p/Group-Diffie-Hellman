// EPOS Smart Data Declarations

#ifndef __smart_data_h
#define __smart_data_h

#include <tstp.h>
#include <periodic_thread.h>
#include <utility/observer.h>

__BEGIN_SYS

class Smart_Data_Common: public _UTIL::Observed
{
public:
    typedef _UTIL::Observed Observed;
    typedef _UTIL::Observer Observer;

    struct DB_Record {
        unsigned long long value;
        unsigned char error;
        long x;
        long y;
        long z;
        unsigned long long t;
        //unsigned char mac[16];
        unsigned char version;
        unsigned int unit;

        friend OStream & operator<<(OStream & os, const DB_Record & d) {
            os << "{va=" << d.value << ",e=" << d.error << ",(" << d.x << "," << d.y << "," << d.z << "),t=" << d.t;//<< ",m=[" << hex << d.mac[0];
            //for(unsigned int i = 1; i < 16; i++)
            //    os << "," << hex << d.mac[i];
            os << dec << "],ve=" << d.version << ",u=" << d.unit << "}";
            return os;
        }
    }__attribute__((packed));

    struct DB_Series {
        unsigned char version;
        unsigned long unit;
        long x;
        long y;
        long z;
        unsigned long r;
        unsigned long long t0;
        unsigned long long t1;

        friend OStream & operator<<(OStream & os, const DB_Series & d) {
            os << "{ve=" << d.version << ",u=" << d.unit << ",(" << d.x << "," << d.y << "," << d.z << ")+" << d.r << ",t=[" << d.t0 << "," << d.t1 << "]}";
            return os;
        }
    }__attribute__((packed));
};

template <typename T>
struct Smart_Data_Type_Wrapper
{
   typedef T Type;
};

// Smart Data encapsulates Transducers (i.e. sensors and actuators), local or remote, and bridges them with TSTP
// Transducers must be Observed objects, must implement either sense() or actuate(), and must define UNIT, NUM, and ERROR.
template<typename Transducer>
class Smart_Data: public Smart_Data_Common, private TSTP::Observer, private Transducer::Observer
{
    friend class Smart_Data_Type_Wrapper<Transducer>::Type; // friend S is OK in C++11, but this GCC does not implements it yet. Remove after GCC upgrade.

private:
    typedef TSTP::Buffer Buffer;
    typedef typename TSTP::Responsive Responsive;
    typedef typename TSTP::Interested Interested;

public:
    static const unsigned int UNIT = Transducer::UNIT;
    static const unsigned int NUM = Transducer::NUM;
    static const unsigned int ERROR = Transducer::ERROR;
    typedef typename TSTP::Unit::Get<NUM>::Type Value;

    enum Mode {
        PRIVATE = 0,
        ADVERTISED = 1,
        COMMANDED = 3,
        CUMULATIVE = 4,
        DISPLAYED = 8,
    };

    static const unsigned int REMOTE = -1;

    typedef RTC::Microsecond Microsecond;

    typedef TSTP::Unit Unit;
    typedef TSTP::Error Error;
    typedef TSTP::Coordinates Coordinates;
    typedef TSTP::Region Region;
    typedef TSTP::Time Time;
    typedef TSTP::Time_Offset Time_Offset;

public:
    // Local data source, possibly advertised to or commanded by the network
    Smart_Data(unsigned int dev, const Microsecond & expiry, const Mode & mode = PRIVATE)
    : _unit(UNIT), _value(0), _error(ERROR), _coordinates(TSTP::here()), _time(TSTP::now()), _expiry(expiry), _device(dev), _mode(mode), _thread(0), _interested(0), _responsive((mode & ADVERTISED) | (mode & COMMANDED) ? new Responsive(this, UNIT, ERROR, expiry, mode & DISPLAYED) : 0) {
        db<Smart_Data>(TRC) << "Smart_Data(dev=" << dev << ",exp=" << expiry << ",mode=" << mode << ")" << endl;
        if(Transducer::POLLING)
            Transducer::sense(_device, this);
        if(Transducer::INTERRUPT)
            Transducer::attach(this);
        if(_responsive)
            TSTP::attach(this, _responsive);
        db<Smart_Data>(INF) << "Smart_Data(dev=" << dev << ",exp=" << expiry << ",mode=" << mode << ") => " << *this << endl;
    }
    // Remote, event-driven (period = 0) or time-triggered data source
    Smart_Data(const Region & region, const Microsecond & expiry, const Microsecond & period = 0, const Mode & mode = PRIVATE)
    : _unit(UNIT), _value(0), _error(ERROR), _coordinates(0), _time(0), _expiry(expiry), _device(REMOTE), _mode(static_cast<Mode>(mode & (~COMMANDED))), _thread(0), _interested(new Interested(this, region, UNIT, TSTP::SINGLE, 0, expiry, period)), _responsive(0) {
        TSTP::attach(this, _interested);
    }

    ~Smart_Data() {
        if(_thread)
            delete _thread;
        if(_interested) {
            TSTP::detach(this, _interested);
            delete _interested;
        }
        if(_responsive) {
            TSTP::detach(this, _responsive);
            delete _responsive;
        }
    }

    DB_Series db_series() {
        DB_Series ret;
        assert(_interested);
        ret.version = 0;
        ret.unit = _unit;
        TSTP::Global_Coordinates c = TSTP::absolute(_interested->region().center);
        ret.x = c.x;
        ret.y = c.y;
        ret.z = c.z;
        ret.r = _interested->region().radius;
        ret.t0 = TSTP::absolute(_interested->region().t0);
        ret.t1 = TSTP::absolute(_interested->region().t1);
        return ret;
    }

    DB_Record db_record() {
        DB_Record ret;
        ret.value = this->operator Value();
        ret.error = error();
        TSTP::Global_Coordinates c = location();
        ret.x = c.x;
        ret.y = c.y;
        ret.z = c.z;
        ret.t = time();
        //for(unsigned int i = 0; i < 16; i++)
        //    ret.mac[i] = 0;
        ret.version = 0;
        ret.unit = unit();
        return ret;
    }

    operator Value() {
        if(expired()) {
            if((_device != REMOTE) && (Transducer::POLLING)) { // Local data source
                Transducer::sense(_device, this); // read sensor
                _time = TSTP::now();
            } else {
                // Other data sources must have called update() timely
                db<Smart_Data>(WRN) << "Smart_Data::get(this=" << this << ",exp=" <<_time +  _expiry << ",val=" << _value << ") => expired!" << endl;
            }
        }
        Value ret = _value;
        if(_mode & CUMULATIVE)
            _value = 0;
        return ret;
    }

    Smart_Data & operator=(const Value & v) {
        if(_device != REMOTE)
            Transducer::actuate(_device, this, v);
        if(_interested)
            _interested->command(v);
        return *this;
    }

    bool expired() const { return TSTP::now() > (_time + _expiry); }

    TSTP::Global_Coordinates location() const { return TSTP::absolute(_coordinates); }
    const Time time() const { return TSTP::absolute(_time); }
    const Error & error() const { return _error; }
    const Unit & unit() const { return _unit; }

    friend Debug & operator<<(Debug & db, const Smart_Data & d) {
        db << "{";
        if(d._device != REMOTE) {
            switch(d._mode) {
            case PRIVATE:    db << "PRI."; break;
            case ADVERTISED: db << "ADV."; break;
            case COMMANDED:  db << "CMD."; break;
            }
            db << "[" << d._device << "]:";
        }
        if(d._thread) db << "ReTT";
        if(d._responsive) db << "ReED";
        if(d._interested) db << "In" << ((d._interested->period()) ? "TT" : "ED");
        db << ":u=" << d._unit << ",v=" << d._value << ",e=" << int(d._error) << ",c=" << d._coordinates << ",t=" << d._time << ",x=" << d._expiry << "}";
        return db;
    }

private:
    void update(TSTP::Observed * obs, int subject, TSTP::Buffer * buffer) {
        TSTP::Packet * packet = buffer->frame()->data<TSTP::Packet>();
        db<Smart_Data>(TRC) << "Smart_Data::update(obs=" << obs << ",cond=" << reinterpret_cast<void *>(subject) << ",data=" << packet << ")" << endl;
        switch(packet->type()) {
        case TSTP::INTEREST: {
            TSTP::Interest * interest = reinterpret_cast<TSTP::Interest *>(packet);
            db<Smart_Data>(INF) << "Smart_Data::update[I]:msg=" << interest << " => " << *interest << endl;
            _responsive->t0(interest->region().t0);
            _responsive->t1(interest->region().t1);
            if(interest->mode() == TSTP::Mode::DELETE) {
                if(_thread) {
                    delete _thread;
                    _thread = 0;
                }
            } else if(interest->period()) {
                if(!_thread)
                    _thread = new Periodic_Thread(interest->period(), &updater, _device, interest->expiry(), this);
                else {
                    if(!interest->period() != _thread->period())
                        _thread->period(interest->period());
                }
            } else {
                Transducer::sense(_device, this);
                _time = TSTP::now();
                _responsive->value(_value);
                _responsive->time(_time);
                _responsive->respond(_time + interest->expiry());
            }
        } break;
        case TSTP::RESPONSE: {
            TSTP::Response * response = reinterpret_cast<TSTP::Response *>(packet);
            db<Smart_Data>(INF) << "Smart_Data:update[R]:msg=" << response << " => " << *response << endl;
            if(response->time() > _time) {
                if(_mode & CUMULATIVE)
                    _value += response->value<Value>();
                else
                    _value = response->value<Value>();
                _error = response->error();
                _coordinates = response->origin();
                _time = response->time();
                db<Smart_Data>(INF) << "Smart_Data:update[R]:this=" << this << " => " << *this << endl;
                notify();
            }
        }
        case TSTP::COMMAND: {
            if(_mode & COMMANDED) {
                TSTP::Command * command = reinterpret_cast<TSTP::Command *>(packet);
                *this = *(command->command<Value>());
                _coordinates = command->origin();
            }
        } break;
        default:
            break;
        }
    }

    void update(typename Transducer::Observed * obs) {
        _time = TSTP::now();
        Transducer::sense(_device, this);
        db<Smart_Data>(TRC) << "Smart_Data::update(this=" << this << ",exp=" << _expiry << ") => " << _value << endl;
        db<Smart_Data>(TRC) << "Smart_Data::update:responsive=" << _responsive << " => " << *reinterpret_cast<TSTP::Response *>(_responsive) << endl;
        notify();
        if(_responsive) {
            _responsive->value(_value);
            _responsive->time(_time);
            _responsive->respond(_time + _expiry);
        }
    }

    static int updater(unsigned int dev, Time_Offset expiry, Smart_Data * data) {
        do {
            Time t = TSTP::now();

            // TODO: The thread should be deleted or suspended when time is up
            if(t < data->_responsive->t1()) {
                Transducer::sense(dev, data);
                data->_time = t;
                data->_responsive->value(data->_value);
                data->_responsive->time(t);
                data->_responsive->respond(t + expiry);

                data->notify();
            }
        } while(Periodic_Thread::wait_next());

        return 0;
    }

private:
    Unit _unit;
    Value _value;
    Error _error;
    Coordinates _coordinates;
    TSTP::Time _time;
    TSTP::Time _expiry;

    unsigned int _device;
    Mode _mode;
    Periodic_Thread * _thread;
    Interested * _interested;
    Responsive * _responsive;
};


// Smart Data Transform and Aggregation functions

class No_Transform
{
public:
    No_Transform() {}

    template<typename T, typename U>
    void apply(T * result, U * source) {
        typename U::Value v = *source;
        *result = v;
    }
};

template<typename T>
class Percent_Transform
{
public:
    Percent_Transform(typename T::Value min, typename T::Value max) : _min(min), _step((max - min) / 100) {}

    template<typename U>
    void apply(U * result, T * source) {
        typename T::Value v = *source;
        if(v < _min)
            *result = 0;
        else {
            v = (v - _min) / _step;
            if(v > 100)
                v = 100;

            *result = v;
        }
    }

private:
    typename T::Value _min;
    typename T::Value _step;
};

template<typename T>
class Inverse_Percent_Transform: private Percent_Transform<T>
{
public:
    Inverse_Percent_Transform(typename T::Value min, typename T::Value max) : Percent_Transform<T>(min, max) {}

    template<typename U>
    void apply(U * result, T * source) {
        typename U::Value r;
        Percent_Transform<T>::apply(&r, source);
        *result = 100 - r;
    }
};

class Sum_Transform
{
public:
    Sum_Transform() {}

    template<typename T, typename ...U>
    void apply(T * result, U * ... sources) {
        *result = sum<T::Value, U...>((*sources)...);
    }

private:
    template<typename T, typename U, typename ...V>
    T sum(const U & s0, const V & ... s) { return s0 + sum<T, V...>(s...); }

    template<typename T, typename U>
    T sum(const U & s) { return s; }
};

class Average_Transform: private Sum_Transform
{
public:
    Average_Transform() {}

    template<typename T, typename ...U>
    void apply(T * result, U * ... sources) {
        typename T::Value r;
        Sum_Transform::apply(&r, sources...);
        *result = r / sizeof...(U);
    }
};


// Smart Data Actuator

template<typename Destination, typename Transform, typename ...Sources>
class Actuator: public Smart_Data_Common::Observer
{
public:
    Actuator(Destination * d, Transform * a, Sources * ... s)
        : _destination(d), _transform(a), _sources{s...}
    {
        attach(s...);
    }
    ~Actuator() {
        unsigned int index = 0;
        detach((reinterpret_cast<Sources*>(_sources[index++]))...);
    }

    void update(Smart_Data_Common::Observed * obs) {
        unsigned int index = 0;
        _transform->apply(_destination, (reinterpret_cast<Sources*>(_sources[index++]))...);
    }

private:
    template<typename T, typename ...U>
    void attach(T * t, U * ... u) { t->attach(this); attach(u...); }
    template<typename T>
    void attach(T * t) { t->attach(this); }

    template<typename T, typename ...U>
    void detach(T * t, U * ... u) { t->detach(this); detach(u...); }
    template<typename T>
    void detach(T * t) { t->detach(this); }

private:
    Destination * _destination;
    Transform * _transform;
    void * _sources[sizeof...(Sources)];
};

__END_SYS

#endif

#include <transducer.h>
