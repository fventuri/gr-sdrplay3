/* -*- c++ -*- */
/*
 * Copyright 2020-2024 Franco Venturi.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SDRPLAY3_RSP_IMPL_H
#define INCLUDED_SDRPLAY3_RSP_IMPL_H

#include <gnuradio/sdrplay3/rsp.h>
#include <sdrplay_api.h>
#include <condition_variable>
#include <queue>

namespace gr {
namespace sdrplay3 {

class rsp_impl : virtual public rsp
{
public:
    // needed to make both gcc/g++ and clang/clang++ compile without errors
    // (because of the position of the 'override' keyboard with a function
    // returning a reference to an array)
    typedef double pair_of_doubles[2];

    /**********************************************************************
     * Structors
     * ********************************************************************/ 
    virtual ~rsp_impl();

    // Sample rate methods
    double set_sample_rate(const double rate,
                           const bool synchronous = false) override;
    double get_sample_rate() const override;
    const pair_of_doubles &get_sample_rate_range() const override;

    // Center frequency methods
    double set_center_freq(const double freq,
                           const bool synchronous = false) override;
    double get_center_freq() const override;
    const pair_of_doubles &get_freq_range() const override;

    // Bandwidth methods
    double set_bandwidth(const double bandwidth) override;
    double get_bandwidth() const override;
    const std::vector<double> get_bandwidths() const override;

    // Gain methods
    const std::vector<std::string> get_gain_names() const override;
    double set_gain(const double gain, const std::string& name,
                    const bool synchronous = false) override;
    double get_gain(const std::string& name) const override;
    const pair_of_doubles &get_gain_range(const std::string& name) const override;
    bool set_gain_mode(bool automatic) override;
    bool get_gain_mode() const override;

    // Miscellaneous stuff
    double set_freq_corr(const double freq) override;
    double get_freq_corr() const override;
    void set_dc_offset_mode(bool enable) override;
    void set_iq_balance_mode(bool enable) override;
    void set_agc_setpoint(double set_point) override;

    // Streaming methods
    virtual bool start() override;
    virtual bool stop() override;

    virtual int work(int noutput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items) override;

    // Stream tags
    void set_stream_tags(bool enable) override;

    // Debug methods
    void set_debug_mode(bool enable) override;
    void set_sample_sequence_gaps_check(bool enable) override;
    void set_show_gain_changes(bool enable) override;

    // Message passing methods
    void handle_command(const pmt::pmt_t& msg);

protected:

    /*! \brief Components common to all RSP devices.
     *
     * \param hwVer the hardware version (i.e. model) of the RSP device
     * \param selector the selector (serial number or GetDevices() index) to identify the hardware
     * \param stream_args stream args (output type, number of channels, etc)
     * \param specific_select optional method to invoke for specific device selection steps
     */
    rsp_impl(const unsigned char hwVer,
             const std::string& selector,
             const struct stream_args_t& stream_args,
             std::function<bool()> specific_select = nullptr);

    io_signature::sptr args_to_io_sig(const struct stream_args_t& args) const;

    void update_sample_rate_and_decimation(double fsHz, int decimation,
                                           sdrplay_api_If_kHzT if_type,
                                           const bool synchronous = false);
    void update_if_streaming(sdrplay_api_ReasonForUpdateT reason_for_update,
                             const bool synchronous = false);
    void update_if_streaming(sdrplay_api_ReasonForUpdateT reason_for_update,
                             sdrplay_api_TunerSelectT tuner,
                             const bool synchronous = false);

    double set_if_gain(const double gain, const bool synchronous = false);
    double set_rf_gain(const double gain, const std::vector<int> rf_gRs,
                       const bool synchronous = false);
    static unsigned char get_closest_LNAstate(const double gain, const std::vector<int> rf_gRs);
    int set_lna_state(const int LNAstate, const std::vector<int> rf_gRs,
                      const bool synchronous = false);
    double get_if_gain() const;
    double get_rf_gain(const std::vector<int> rf_gRs) const;
    int get_lna_state() const;
    const double (&get_if_gain_range() const)[2];
    const double (&get_rf_gain_range(const std::vector<int> rf_gRs) const)[2];
    const int (&get_lna_state_range(const std::vector<int> rf_gRs) const)[2];
    virtual const std::vector<int> rf_gr_values() const = 0;

    bool start_api_init();
    void add_stream_tags(size_t start, size_t end, int noutput_items,
                         int stream_index);

    // callback functions
    virtual void event_callback(sdrplay_api_EventT eventId,
                                sdrplay_api_TunerSelectT tuner,
                                sdrplay_api_EventParamsT *params);

    virtual void print_device_config() const;

private:
    bool rsp_select(const unsigned char hwVer, const std::string& selector);
    sdrplay_api_Bw_MHzT get_auto_bandwidth() const;

    // callback functions
    static void stream_A_callback(short *xi, short *xq,
                                  sdrplay_api_StreamCbParamsT *params,
                                  unsigned int numSamples, unsigned int reset,
                                  void *cbContext);
    static void stream_B_callback(short *xi, short *xq,
                                  sdrplay_api_StreamCbParamsT *params,
                                  unsigned int numSamples, unsigned int reset,
                                  void *cbContext);
    static void event_callback(sdrplay_api_EventT eventId,
                               sdrplay_api_TunerSelectT tuner,
                               sdrplay_api_EventParamsT *params,
                               void *cbContext);
    void stream_callback(short *xi, short *xq,
                         sdrplay_api_StreamCbParamsT *params,
                         unsigned int numSamples, unsigned int reset,
                         int stream_index,
                         sdrplay_api_RxChannelParamsT *rx_channel);

    // ring buffers to transfer data from the stream callbacks to work()
    // RingBufferSize must be a power of 2 to simplify wrap-around
    constexpr static unsigned int RingBufferSize = 65536;
    constexpr static unsigned int RingBufferMask = RingBufferSize - 1;
    struct {
        short* xi;
        short* xq;
        uint64_t head;
        uint64_t tail;
        std::condition_variable empty;
        std::condition_variable overflow;
        std::mutex mtx;
    } ring_buffers[2];

    // changes to sample rate, fequency, and gain reduction reported by
    // RX callback
    int sample_rate_changed;
    int frequency_changed;
    int gain_reduction_changed;
    // synchronous updates
    std::mutex value_changed_mutex;
    std::condition_variable value_changed_cv;

    constexpr static std::chrono::milliseconds update_timeout = std::chrono::milliseconds(500);

    // param changes as stream tags
    bool stream_tags;
    enum ParamChangeType {pct_rate=1, pct_freq=2, pct_gains=3};
    struct param_change {
        unsigned long offset;
        enum ParamChangeType pctype;
        union {
            double rate;
            double freq;
            int gains[2];
        };
    };
    std::queue<struct param_change> param_changes[2];
    std::mutex param_change_mutex[2];

    bool sample_sequence_gaps_check;
    bool show_gain_changes;

protected:
    sdrplay_api_DeviceT device;
    sdrplay_api_DeviceParamsT *device_params;
    sdrplay_api_RxChannelParamsT *rx_channel_params;
    double sample_rate;
    enum RunStatus {idle=0, init=1, in_transition=2, streaming=3};
    RunStatus run_status;
    int nchannels;
    enum OutputType {fc32=1, sc16=2};
    enum OutputType output_type;

    struct _output_type {
        enum OutputType output_type;
        size_t size;
    };
    static const std::map<std::string, struct _output_type> output_types;
};

} // namespace sdrplay3
} // namespace gr

#endif /* INCLUDED_SDRPLAY3_RSP_IMPL_H */
