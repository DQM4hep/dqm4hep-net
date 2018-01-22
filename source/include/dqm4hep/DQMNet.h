/// \file DQMNet.h
/*
 *
 * DQMNet.h header template automatically generated by a class generator
 * Creation date : dim. d�c. 4 2016
 *
 * This file is part of DQM4HEP libraries.
 *
 * DQM4HEP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 *
 * DQM4HEP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DQM4HEP.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */


#ifndef DQMNET_H
#define DQMNET_H

#include <memory>
#include <string>
#include <limits>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <typeinfo>
#include <cstring>

#include "json/json.h"

namespace dqm4hep {

  namespace net {

    struct NullBuffer
    {
      static const char buffer[2];
      static const size_t size;
    };

    class RawBuffer
    {
    public:
      ///< Default constructor
      RawBuffer() { this->adopt(nullptr, 0); }

      RawBuffer(const RawBuffer&) = delete;
      RawBuffer(RawBuffer&&) = delete;
      RawBuffer &operator=(const RawBuffer&) = delete;
      RawBuffer &&operator=(RawBuffer &&) = delete;

      inline const char *begin() const { return m_pBuffer; }
      inline const char *end() const { return m_pBuffer+m_size; }
      inline size_t size() const { return m_size; }
      inline void adopt(const char *buffer, size_t size) { m_pBuffer = buffer; m_size = size; }
    private:
      const char *m_pBuffer = nullptr;
      size_t m_size = 0;
    };

    class BufferModel
    {
    public:
      ///< Constructor
      BufferModel() {}
      ///< Get the raw buffer
      const RawBuffer &raw() const { return m_rawBuffer; }
      ///< Handle the buffer. The buffer is still owned by the user
      void handle(const char *buffer, size_t size) { m_rawBuffer.adopt(buffer, size); }
    protected:
      RawBuffer      m_rawBuffer;
    };

    template <typename T>
    class BufferModelT : public BufferModel
    {
    public:
      ///< Constructor
      BufferModelT() { m_rawBuffer.adopt((const char*)&m_value, sizeof(m_value)); }
      ///< Copy the value for further use
      void copy(const T &value) { m_value = T(value); m_rawBuffer.adopt((const char*)&m_value, sizeof(m_value)); }
      ///< Move the value for further use
      void move(T &&value) { m_value = std::move(value); m_rawBuffer.adopt((const char*)&m_value, sizeof(m_value)); }
    private:
      T           m_value;
    };

    template <>
    class BufferModelT<std::string> : public BufferModel
    {
    public:
      ///< Constructor
      BufferModelT() { m_value.assign(NullBuffer::buffer, NullBuffer::size); m_rawBuffer.adopt(m_value.c_str(), m_value.size()); }
      ///< Copy the value for further use
      void copy(const std::string &value) { m_value = value; m_rawBuffer.adopt(m_value.c_str(), m_value.size()); }
      ///< Move the value for further use
      void move(std::string &&value) { m_value = std::move(value); m_rawBuffer.adopt(m_value.c_str(), m_value.size()); }
    private:
      std::string          m_value;
    };

    typedef std::shared_ptr<BufferModel> BufferModelPtr;

    ///< For handling client-size buffer in user callbacks ...
    class Buffer
    {
    public:
      ///< Default constructor
      inline Buffer()
      {
        this->adopt(NullBuffer::buffer, NullBuffer::size);
      }

      template <typename T>
      inline std::shared_ptr<BufferModelT<T>> createModel() const
      {
        return std::make_shared<BufferModelT<T>>();
      }

      inline std::shared_ptr<BufferModel> createModel() const
      {
        return std::make_shared<BufferModel>();
      }

      inline void setModel(std::shared_ptr<BufferModel> model)
      {
        if( ! model )
          return;

        m_model = model;
      }

      inline const char *begin() const
      {
        return m_model->raw().begin();
      }

      inline const char *end() const
      {
        return m_model->raw().end();
      }

      inline size_t size() const
      {
        return m_model->raw().size();
      }

      inline void adopt(const char *buffer, size_t size)
      {
        auto model = this->createModel();
        model->handle(buffer, size);
        this->setModel(model);
      }
      
      inline BufferModelPtr model() const
      {
        return m_model;
      }
      
      Buffer(Buffer&& buffer)
      {
        m_model = std::move(buffer.m_model);
      }

      Buffer(const Buffer&) = delete;
      Buffer &operator=(const Buffer&) = delete;
      Buffer &&operator=(Buffer &&) = delete;

    private:
      BufferModelPtr     m_model;
    };

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    /**
     *  @brief SignalBase class.
     *         Base class to store callback function
     */
    template <typename ...Args>
    class SignalBase
    {
    public:
      typedef std::vector<SignalBase<Args...> *>  Vector;

      /**
       *  @brief  Destructor
       */
      virtual ~SignalBase() {}

      /**
       *  @brief  Process the callback
       */
      virtual void process(Args ...args) = 0;
    };

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    /**
     *  @brief  SignalBase specialization class for void type (no arg)
     */
    template <>
    class SignalBase<void>
    {
    public:
      typedef std::vector<SignalBase<void> *>  Vector;

      /**
       *  @brief  Destructor
       */
      virtual ~SignalBase() {}

      /**
       *  @brief  Process the callback
       */
      virtual void process() = 0;
    };

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    /**
     *  @brief  SignalT class.
     */
    template <typename T, typename ...Args>
    class SignalT : public SignalBase<Args ...>
    {
    public:
      typedef void (T::*Function)(Args...args);

      /**
       *  @brief  Constructor with
       */
      SignalT(T *pClass, Function function);

      /**
       *  @brief  Process the callback
       */
      void process(Args ...args);

      /**
       *  @brief  Get the class instance
       */
      const T *getClass() const;

      /**
       *  @brief  Get the callback class method
       */
      const Function getFunction() const;

    private:
      T                 *m_pClass;             ///< The class instance
      Function           m_function;           ///< The callback class method
    };

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    /**
     *  @brief  SignalT specialization for void type (no arg)
     *  @see    SignalT
     */
    template <typename T>
    class SignalT<T, void> : public SignalBase<void>
    {
    public:
      typedef void (T::*Function)();
      SignalT(T *pClass, Function function);
      void process();
      const T *getClass() const;
      const Function getFunction() const;

    private:
      T                 *m_pClass;
      Function           m_function;
    };

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    /**
     *  @brief  SignalT specialization for void type (no class)
     *  @see    SignalT
     */
     template <typename ...Args>
     class SignalT<void, Args...> : public SignalBase<Args ...>
     {
     public:
       typedef void (*Function)(Args...args);
       SignalT(Function function);
       void process(Args ...args);
       const void *getClass() const;
       const Function getFunction() const;

     private:
       Function           m_function;           ///< The callback class method
     };

     //----------------------------------------------------------------------------------
     //----------------------------------------------------------------------------------

     /**
      *  @brief  SignalT specialization for void type (no class and no arg)
      *  @see    SignalT
      */
     template <>
      class SignalT<void, void> : public SignalBase<void>
      {
      public:
        typedef void (*Function)();
        SignalT(Function function);
        void process();
        const void *getClass() const;
        const Function getFunction() const;

      private:
        Function           m_function;           ///< The callback class method
      };

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    /**
     *  @brief  Signal class. User interface class to use signals
     */
    template <typename ...Args>
    class Signal
    {
    public:
      /**
       *  @brief  Destructor
       */
      ~Signal();

      /**
       *  @brief  Process the signal. Calls all registered class->method(args...) signal handlers
       */
      void process(Args ...args);

      /**
       *  @brief  Connect a class method to this signal
       */
      template <typename T, typename S>
      bool connect(T *pClass, S function);

      /**
       *  @brief  Disconnect all class methods for a target class instance
       */
      template <typename T>
      bool disconnect(T *pClass);

      /**
       *  @brief  Disconnect a target class instance method
       */
      template <typename T, typename S>
      bool disconnect(T *pClass, S function);

      /**
       *  @brief  Disconnect all class methods
       */
      void disconnectAll();

      /**
       *  @brief  Whether the class instance has at least one connection
       *  with this signal
       */
      template <typename T>
      bool isConnected(T *pClass) const;

      /**
       *  @brief  Whether the target class method is connected to this signal
       */
      template <typename T, typename S>
      bool isConnected(T *pClass, S function) const;

      /**
       *  @brief  Whether this signal has at least one registered class method
       */
      bool hasConnection() const;

    private:
      typename SignalBase<Args...>::Vector           m_callbacks;
    };

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    template <>
    class Signal<void>
    {
    public:
      ~Signal();
      void process();

      template <typename T, typename S>
      bool connect(T *pClass, S function);


      template <typename T>
      bool disconnect(T *pClass);

      template <typename T, typename S>
      bool disconnect(T *pClass, S function);

      void disconnectAll();

      template <typename T>
      bool isConnected(T *pClass) const;

      template <typename T, typename S>
      bool isConnected(T *pClass, S function) const;

      bool hasConnection() const;

    private:
      typename SignalBase<void>::Vector           m_callbacks;
    };

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    template <typename ...Args>
    inline Signal<Args ...>::~Signal()
    {
      this->disconnectAll();
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    inline void Signal<Args ...>::process(Args ...args)
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
        (*iter)->process(args...);
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    template <typename T, typename S>
    inline bool Signal<Args ...>::connect(T *pClass, S function)
    {
      // check for existing connection
      if(this->isConnected(pClass))
        return false;

      // add the callback
      m_callbacks.push_back(new SignalT<T, Args...>(pClass, function));

      return true;
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    template <typename T>
    inline bool Signal<Args ...>::disconnect(T *pClass)
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        SignalT<T, Args...> *pCallBackT(dynamic_cast<SignalT<T, Args...> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass)
        {
          delete pCallBackT;
          m_callbacks.erase(iter);
          return true;
        }
      }

      return false;
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    template <typename T, typename S>
    inline bool Signal<Args ...>::disconnect(T *pClass, S function)
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        SignalT<T, Args...> *pCallBackT(dynamic_cast<SignalT<T, Args...> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass && pCallBackT->getFunction() == function)
        {
          delete pCallBackT;
          m_callbacks.erase(iter);
          return true;
        }
      }

      return false;
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    inline void Signal<Args ...>::disconnectAll()
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
        delete *iter;

      m_callbacks.clear();
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    template <typename T>
    inline bool Signal<Args ...>::isConnected(T *pClass) const
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        const SignalT<T, Args...> *pCallBackT(dynamic_cast<const SignalT<T, Args...> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass)
          return true;
      }

      return false;
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    template <typename T, typename S>
    inline bool Signal<Args...>::isConnected(T *pClass, S function) const
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        const SignalT<T, Args...> *pCallBackT(dynamic_cast<const SignalT<T, Args...> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass && pCallBackT->getFunction() == function)
          return true;
      }

      return false;
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    inline bool Signal<Args ...>::hasConnection() const
    {
      return (!m_callbacks.empty());
    }

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    inline Signal<void>::~Signal()
    {
      this->disconnectAll();
    }

    //----------------------------------------------------------------------------------

    inline void Signal<void>::process()
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
        (*iter)->process();
    }

    //----------------------------------------------------------------------------------

    template <typename T, typename S>
    inline bool Signal<void>::connect(T *pClass, S function)
    {
      // check for existing connection
      if(this->isConnected(pClass))
        return false;

      // add the callback
      m_callbacks.push_back(new SignalT<T, void>(pClass, function));

      return true;
    }

    //----------------------------------------------------------------------------------

    template <typename T>
    inline bool Signal<void>::disconnect(T *pClass)
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        SignalT<T, void> *pCallBackT(dynamic_cast<SignalT<T, void> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass)
        {
          delete pCallBackT;
          m_callbacks.erase(iter);
          return true;
        }
      }

      return false;
    }

    //----------------------------------------------------------------------------------

    template <typename T, typename S>
    inline bool Signal<void>::disconnect(T *pClass, S function)
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        SignalT<T, void> *pCallBackT(dynamic_cast<SignalT<T, void> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass && pCallBackT->getFunction() == function)
        {
          delete pCallBackT;
          m_callbacks.erase(iter);
          return true;
        }
      }

      return false;
    }

    //----------------------------------------------------------------------------------

    inline void Signal<void>::disconnectAll()
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
        delete *iter;

      m_callbacks.clear();
    }

    //----------------------------------------------------------------------------------

    template <typename T>
    inline bool Signal<void>::isConnected(T *pClass) const
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        const SignalT<T, void> *pCallBackT(dynamic_cast<const SignalT<T, void> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass)
          return true;
      }

      return false;
    }

    //----------------------------------------------------------------------------------

    template <typename T, typename S>
    inline bool Signal<void>::isConnected(T *pClass, S function) const
    {
      for(auto iter = m_callbacks.begin(), endIter = m_callbacks.end() ; endIter != iter ; ++iter)
      {
        const SignalT<T, void> *pCallBackT(dynamic_cast<const SignalT<T, void> *>(*iter));

        if(!pCallBackT)
          continue;

        if(pCallBackT->getClass() == pClass && pCallBackT->getFunction() == function)
          return true;
      }

      return false;
    }

    //----------------------------------------------------------------------------------

    inline bool Signal<void>::hasConnection() const
    {
      return (!m_callbacks.empty());
    }

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    template <typename T, typename ...Args>
    inline SignalT<T, Args...>::SignalT(T *pClass, Function function) :
      m_pClass(pClass),
      m_function(function)
    {
      /* nop */
    }

    //----------------------------------------------------------------------------------

    template <typename T, typename ...Args>
    inline void SignalT<T, Args...>::process(Args... args)
    {
      (m_pClass->*m_function)(args...);
    }

    //----------------------------------------------------------------------------------

    template <typename T, typename ...Args>
    inline const T *SignalT<T, Args...>::getClass() const
    {
      return m_pClass;
    }

    //----------------------------------------------------------------------------------

    template <typename T, typename ...Args>
    inline const typename SignalT<T, Args...>::Function SignalT<T, Args...>::getFunction() const
    {
      return m_function;
    }

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    template <typename T>
    inline SignalT<T, void>::SignalT(T *pClass, Function function) :
      m_pClass(pClass),
      m_function(function)
    {
      /* nop */
    }

    //----------------------------------------------------------------------------------

    template <typename T>
    inline void SignalT<T, void>::process()
    {
      (m_pClass->*m_function)();
    }

    //----------------------------------------------------------------------------------

    template <typename T>
    inline const T *SignalT<T, void>::getClass() const
    {
      return m_pClass;
    }

    //----------------------------------------------------------------------------------

    template <typename T>
    inline const typename SignalT<T, void>::Function SignalT<T, void>::getFunction() const
    {
      return m_function;
    }

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    template <typename ...Args>
    inline SignalT<void, Args...>::SignalT(Function function) :
      m_function(function)
    {
      /* nop */
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    inline void SignalT<void, Args...>::process(Args... args)
    {
      (*m_function)(args...);
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    inline const void *SignalT<void, Args...>::getClass() const
    {
      return nullptr;
    }

    //----------------------------------------------------------------------------------

    template <typename ...Args>
    inline const typename SignalT<void, Args...>::Function SignalT<void, Args...>::getFunction() const
    {
      return m_function;
    }

    //----------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------

    // template <>
    inline SignalT<void, void>::SignalT(Function function) :
      m_function(function)
    {
      /* nop */
    }

    //----------------------------------------------------------------------------------

    // template <>
    inline void SignalT<void, void>::process()
    {
      (*m_function)();
    }

    //----------------------------------------------------------------------------------

    // template <>
    inline const void *SignalT<void, void>::getClass() const
    {
      return nullptr;
    }

    //----------------------------------------------------------------------------------

    // template <>
    inline const typename SignalT<void, void>::Function SignalT<void, void>::getFunction() const
    {
      return m_function;
    }

  }

}

#endif  //  DQMNET_H
