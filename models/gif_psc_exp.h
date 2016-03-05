/*
 *  gif_psc_exp.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef gif_psc_exp_H
#define gif_psc_exp_H

#include "nest.h"
#include "event.h"
#include "archiving_node.h"
#include "ring_buffer.h"
#include "connection.h"
#include "poisson_randomdev.h"
#include "gamma_randomdev.h"
#include "universal_data_logger.h"

namespace nest{

  /* BeginDocumentation
    Name: gif_psc_exp - Current based generalized integrate-and-fire neuron model according to Mensi et al. (2012) 
    and Pozzorini et al. (2015)

    Description:

    gif_psc_exp is the generalized integrate-and-fire neuron according to Mensi et al. (2012)
    and Pozzorini et al. (2015), with exponential shaped postsynaptic currents.

    This model features both an adaptation current and a dynamic threshold for spike-frequency
    adaptation. The membrane potential (V) is described by the differential equation:

    C*dV(t)/dt = -g_L*(V(t)-E_L) - etta_1(t) - etta_2(t) - ... - etta_n(t) + I(t)

    where each etta_i is a spike triggered current (stc), and the neuron model can have arbitrary number of them.
    Dynamic of each etta_i is described by:

    Tau_etta_i*d{etta_i}/dt = -etta_i

    and in case of spike emission, its value increased by a constant (which can be positive or negative): 

    etta_i = etta_i + q_etta_i  (in case of spike emission).

    Neuron produces spikes STOCHASTICALLY according to a point process with the firing intensity:

    lambda(t) = lambda0 * exp[(V(t)-V_T(t)/delta_u)]

    where V_T(t) is a time-dependent firing threshold:

    V_T(t) = V_T_star + gamma_1(t) + gamma_2(t) + ... + gamma_m(t)

    where gamma_i is a kernel of spike-frequency adaptation (sfa), and the neuron model can have arbitrary number of them.
    Dynamic of each gamma_i is described by:

    Tau_gamma_i*d{gamma_i}/dt = -gamma_i

    and in case of spike emission, its value increased by a constant (which can be positive or negative): 

    gamma_i = gamma_i + q_gamma_i  (in case of spike emission).

    In the source code and parameter names we use stc and sfa, respectively instead of etta and gamma.


    References:
    
    [1] Mensi, S., Naud, R., Pozzorini, C., Avermann, M., Petersen, C. C., & Gerstner, W. (2012). Parameter 
    extraction and classification of three cortical neuron types reveals two distinct adaptation mechanisms. 
    Journal of Neurophysiology, 107(6), 1756-1775.

    [2] Pozzorini, C., Mensi, S., Hagens, O., Naud, R., Koch, C., & Gerstner, W. (2015). Automated 
    High-Throughput Characterization of Single Neurons by Means of Simplified Spiking Models. PLoS 
    Comput Biol, 11(6), e1004275.


    Parameters:
    The following parameters can be set in the status dictionary.

    Membrane Parameters:
      C_m        double - Capacity of the membrane in pF
      t_ref      double - Duration of refractory period in ms.
      V_reset    double - Reset value after a spike in mV.
      E_L        double - Leak reversal potential in mV.
      g_L        double - Leak conductance in nS.
      I_e        double - Constant external input current in pA.

    Spike adaptation and firing intensity parameters:
      q_stc      vector of double - Values added to spike triggered currents (stc) after each spike emission in nA.
      tau_stc    vector of double - Time constants of stc variables in ms.
      q_sfa      vector of double - Values added to spike-frequency adaptation (sfa) after each spike emission in mV.
      tau_sfa    vector of double - Time constants of sfa variables in ms.
      delta_u    double - Stochasticity level in mV.
      lambda0    double - Stochastic intensity at firing threshold V_T in Hz.
      v_t_star   double - Minimum threshold in mV

    Synaptic parameters
      tau_syn_ex double - Time constant of the excitatory synaptic current in ms (exp function).
      tau_syn_in double - Time constant of the inhibitory synaptic current in ms (exp function).



    Sends: SpikeEvent

    Receives: SpikeEvent, CurrentEvent, DataLoggingRequest
  
    Author: March 2016, Setareh
    SeeAlso: pp_psc_delta, gif_psc_exp_multisynapse, gif_cond_exp, gif_cond_exp_multisynapse

  */

  class gif_psc_exp:
  public Archiving_Node
  {

  public:

    gif_psc_exp();
    gif_psc_exp(const gif_psc_exp&);

    /**
     * Import sets of overloaded virtual functions.
     * @see Technical Issues / Virtual Functions: Overriding, Overloading, and Hiding
     */
    using Node::handle;
    using Node::handles_test_event;

    port send_test_event(Node&, rport, synindex, bool);

    void handle(SpikeEvent &);
    void handle(CurrentEvent &);
    void handle(DataLoggingRequest &);

    port handles_test_event(SpikeEvent&, rport);
    port handles_test_event(CurrentEvent&, rport);
    port handles_test_event(DataLoggingRequest&, rport);


    void get_status(DictionaryDatum &) const;
    void set_status(const DictionaryDatum &);

  private:

    void init_state_(const Node& proto);
    void init_buffers_();
    void calibrate();

    void update(Time const &, const long_t, const long_t);

    // The next two classes need to be friends to access the State_ class/member
    friend class RecordablesMap<gif_psc_exp>;
    friend class UniversalDataLogger<gif_psc_exp>;

    // ----------------------------------------------------------------

    /**
     * Independent parameters of the model.
     */
    struct Parameters_ {

    double_t g_L_;
    double_t E_L_;
    double_t V_reset_;
    double_t delta_u_;
    double_t v_t_star_;
    double_t lambda0_;

      /** Refractory period in ms. */
    double_t t_ref_;

      /** Membrane capacitance in pF. */
      double_t c_m_;

      /** List of spike triggered current time constant in ms. */
      std::vector<double_t> tau_stc_;

      /** List of spike triggered current jumps in nA. */
      std::vector<double_t> q_stc_;

      /** List of adaptive threshold time constant in ms. */
      std::vector<double_t> tau_sfa_;

      /** List of adaptive threshold jumps in mV. */
      std::vector<double_t> q_sfa_;

      /** Time constant of excitatory synaptic current in ms. */
      double_t tau_ex_;

      /** Time constant of inhibitory synaptic current in ms. */
      double_t tau_in_;


      /** External DC current. */
      double_t I_e_;

      Parameters_();  //!< Sets default parameter values

      void get(DictionaryDatum&) const;  //!< Store current values in dictionary
      void set(const DictionaryDatum&);  //!< Set values from dictionary
    };

    // ----------------------------------------------------------------

    /**
     * State variables of the model.
     */
    struct State_ {
      double_t     y0_; //!< This is piecewise constant external current
      double_t     y3_; //!< This is the membrane potential RELATIVE TO RESTING POTENTIAL.
      double_t     q_;  //!< This is the change of the 'threshold' due to adaptation.
      double_t     stc_; // Spike triggered current.

      std::vector<double_t>  q_sfa_elems_; // Vector of adaptation parameters. 
      std::vector<double_t>  q_stc_elems_; // Vector of spike triggered parameters.

      double_t i_syn_ex_; // postsynaptic current for exc.
      double_t i_syn_in_; // postsynaptic current for inh.

      int_t r_ref_;  // absolute refractory counter (no membrane potential propagation)

      bool initialized_; // it is true if the vectors are initialized
      bool add_stc_sfa_; // in case of true, the stc and sfa ampplitudes should be added

      State_();  //!< Default initialization

      void get(DictionaryDatum&, const Parameters_&) const;
      void set(const DictionaryDatum&, const Parameters_&);
    };

    // ----------------------------------------------------------------

    /**
     * Buffers of the model.
     */
    struct Buffers_ {
      Buffers_(gif_psc_exp &);
      Buffers_(const Buffers_ &, gif_psc_exp &);

      /** buffers and sums up incoming spikes/currents */
      RingBuffer spikes_ex_;
      RingBuffer spikes_in_;
      RingBuffer currents_;

      //! Logger for all analog data
      UniversalDataLogger<gif_psc_exp> logger_;
    };

    // ----------------------------------------------------------------

    /**
     * Internal variables of the model.
     */
    struct Variables_ {

      double_t P30_;
      double_t P33_;
      double_t P31_;
      double_t P11ex_;
      double_t P11in_;
      double_t P21ex_;
      double_t P21in_;
      std::vector<double_t> Q33_;	//for sfa
      std::vector<double_t> Q44_;	//for stc


      double_t h_;              //!< simulation time step in ms

      librandom::RngPtr rng_; // random number generator of my own thread
      librandom::PoissonRandomDev poisson_dev_;  // random deviate generator
      librandom::GammaRandomDev gamma_dev_;  // random deviate generator


      int_t RefractoryCounts_;
    };

    // Access functions for UniversalDataLogger -----------------------

    //! Read out the real membrane potential
    double_t get_V_m_() const { return S_.y3_ ;}

    //! Read out the adaptive threshold potential
    double_t get_E_sfa_() const { return S_.q_; }

    double_t
    get_input_currents_ex_() const
    {
      return S_.i_syn_ex_;
    }
 
    double_t
    get_input_currents_in_() const
    {
    return S_.i_syn_in_;
    }

    // ----------------------------------------------------------------

    /**
     * @defgroup iaf_psc_alpha_data
     * Instances of private data structures for the different types
     * of data pertaining to the model.
     * @note The order of definitions is important for speed.
     * @{
     */
    Parameters_ P_;
    State_      S_;
    Variables_  V_;
    Buffers_    B_;
    /** @} */

    //! Mapping of recordables names to access functions
    static RecordablesMap<gif_psc_exp> recordablesMap_;
  };

  inline
port gif_psc_exp::send_test_event(Node& target, rport receptor_type, synindex, bool)
{
  SpikeEvent e;
  e.set_sender(*this);
  
  return target.handles_test_event(e, receptor_type);
}


inline
port gif_psc_exp::handles_test_event(SpikeEvent&, rport receptor_type)
{
  if (receptor_type != 0)
    throw UnknownReceptorType(receptor_type, get_name());
  return 0;
}

inline
port gif_psc_exp::handles_test_event(CurrentEvent&, rport receptor_type)
{
  if (receptor_type != 0)
    throw UnknownReceptorType(receptor_type, get_name());
  return 0;
}

inline
port gif_psc_exp::handles_test_event(DataLoggingRequest &dlr, 
				    rport receptor_type)
{
  if (receptor_type != 0)
    throw UnknownReceptorType(receptor_type, get_name());
  return B_.logger_.connect_logging_device(dlr, recordablesMap_);
}

inline
void gif_psc_exp::get_status(DictionaryDatum &d) const
{
  P_.get(d);
  S_.get(d, P_);
  Archiving_Node::get_status(d);
  (*d)[names::recordables] = recordablesMap_.get_list();
}

inline
void gif_psc_exp::set_status(const DictionaryDatum &d)
{
  Parameters_ ptmp = P_;  // temporary copy in case of errors
  ptmp.set(d);                       // throws if BadProperty
  State_      stmp = S_;  // temporary copy in case of errors
  stmp.set(d, ptmp);                 // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  Archiving_Node::set_status(d);

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace

#endif /* #ifndef gif_psc_exp_H */
