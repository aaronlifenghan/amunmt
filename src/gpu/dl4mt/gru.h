#pragma once

#include "../mblas/matrix.h"

namespace GPU {

template <class Weights>
class SlowGRU {
  public:
    SlowGRU(const Weights& model)
    : w_(model) {}
          
    void GetNextState(mblas::Matrix& NextState,
                      const mblas::Matrix& State,
                      const mblas::Matrix& Context) const {
      using namespace mblas;
      
      const size_t cols = GetStateLength();
      
      // @TODO: Optimization
      // @TODO: Launch streams to perform GEMMs in parallel
      // @TODO: Join matrices and perform single GEMM --------
      Prod(RU_, Context, w_.W_);
      Prod(H_,  Context, w_.Wx_);
      // -----------------------------------------------------
      
      // @TODO: Join matrices and perform single GEMM --------
      Prod(Temp1_, State, w_.U_);
      Prod(Temp2_, State, w_.Ux_);        
      // -----------------------------------------------------
      
      // @TODO: Organize into one kernel ---------------------
      BroadcastVec(_1 + _2, RU_, w_.B_); // Broadcasting row-wise
      Element(Logit(_1 + _2), RU_, Temp1_);
      Slice(R_, RU_, 0, cols);
      Slice(U_, RU_, 1, cols);
      
      BroadcastVec(_1 + _2, H_,    w_.Bx1_); // Broadcasting row-wise
      BroadcastVec(_1 + _2, Temp2_, w_.Bx2_); // Broadcasting row-wise
      
      Element(Tanh(_1 + _2 * _3), H_, R_, Temp2_);
      Element((1.0 - _1) * _2 + _1 * _3, U_, H_, State);
      // -----------------------------------------------------
      
      Swap(NextState, U_);
    }
    
    size_t GetStateLength() const {
      return w_.U_.GetShape().rows;
    }
    
  private:
    // Model matrices
    const Weights& w_;
    
    // reused to avoid allocation
    mutable mblas::Matrix RU_;
    mutable mblas::Matrix R_;
    mutable mblas::Matrix U_;
    mutable mblas::Matrix H_;
    mutable mblas::Matrix Temp1_;
    mutable mblas::Matrix Temp2_;
};

__global__ void gElementwiseOps(float* out,
                                const float* state,
                                const float* ruh,
                                const float* t,
                                const float* b,
                                const float* bx1,
                                const float* bx2,
                                size_t rows, size_t cols);

template <class Weights>
class FastGRU {
  public:
    FastGRU(const Weights& model)
    : w_(model) {
      /*for(int i = 0; i < 4; ++i) {
        cudaStreamCreate(&s_[i]);
        cublasCreate(&h_[i]);
        cublasSetStream(h_[i], s_[i]);            
      }*/
      
      using namespace mblas;
      Transpose(WWx_, w_.W_);
      Matrix WxT;
      Transpose(WxT, w_.Wx_);
      Concat(WWx_, WxT);
      Transpose(WWx_);
      
      Transpose(UUx_, w_.U_);
      Matrix UxT;
      Transpose(UxT, w_.Ux_);
      Concat(UUx_, UxT);
      Transpose(UUx_);
    }
          
    void GetNextState(mblas::Matrix& NextState,
                      const mblas::Matrix& State,
                      const mblas::Matrix& Context) const {
      using namespace mblas;
      
      const size_t cols = GetStateLength();
      /*
      std::cerr << "BEFORE cols=" << cols << " "
    		  << NextState.GetShape().Debug() << " "
    		  << State.GetShape().Debug() << " "
    		  << Context.GetShape().Debug() << " "
    		  << RUH_.GetShape().Debug() << " "
    		  << WWx_.GetShape().Debug() << " "
    		  << UUx_.GetShape().Debug() << " "
    		  << std::endl;
      */
      Prod(RUH_, Context, WWx_);
      Prod(Temp_, State, UUx_);
      ElementwiseOps(NextState, State, RUH_, Temp_);
      /*
      std::cerr << "AFTER cols=" << cols << " "
    		  << NextState.GetShape().Debug() << " "
    		  << State.GetShape().Debug() << " "
    		  << Context.GetShape().Debug() << " "
    		  << RUH_.GetShape().Debug() << " "
    		  << WWx_.GetShape().Debug() << " "
    		  << UUx_.GetShape().Debug() << " "
    		  << std::endl;
      */
    }
      
      
    void ElementwiseOps(mblas::Matrix& NextState,
                        const mblas::Matrix& State,
                        const mblas::Matrix& RUH,
                        const mblas::Matrix& Temp) const {
      const size_t rows = State.GetShape().rows;
      const size_t cols = State.GetShape().cols;
      NextState.Resize(rows, cols);
      
      int blocks  = std::min(MAX_BLOCKS, (int)rows);
      int threads = std::min(MAX_THREADS, (int)cols);
      gElementwiseOps<<<blocks, threads>>>(NextState.data(), State.data(),
                                          RUH.data(),
                                          Temp.data(),
                                          w_.B_.data(), w_.Bx1_.data(), w_.Bx2_.data(),
                                          rows, cols);
      cudaStreamSynchronize(0);
    }
    
    size_t GetStateLength() const {
      return w_.U_.GetShape().rows;
    }

    
  private:
    // Model matrices
    const Weights& w_;
    
    cublasHandle_t h_[4];
    cudaStream_t s_[4];
        
    // reused to avoid allocation
    mutable mblas::Matrix WWx_;
    mutable mblas::Matrix UUx_;
    
    mutable mblas::Matrix RUH_;
    mutable mblas::Matrix Temp_;
};

template<class T>
using GRU = FastGRU<T>;

}

