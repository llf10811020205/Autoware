#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <thread>
#include <vector>

#include <cassert>
#include <mutex>

#include <state_context.hpp>

#include <state.hpp>
#include <state_common.hpp>

/**
 *
 * @file StateContext.cpp
 * @brief state context class
 * @author Yusuke Fujii
 * @date  2017/07/25
 *
 **/

namespace state_machine
{
void StateContext::update(void)
{
	for(auto &&p : HolderList){
		if(*p){
			(*p)->update();
		}
	}

}


void StateContext::showCurrentStateName(void)
{
  for (auto &&p : HolderList) {
    if (*p)
    {
      (*p)->showStateName();
    }
  }

#if 0
  if (sub_state)
    sub_state->showStateName();
  if (sub_sub_state)
    sub_sub_state->showStateName();
#endif
  std::cout << std::endl;
}


bool StateContext::isDifferentState(BaseState *_state_a, BaseState *_state_b)
{
	return _state_a == _state_b;
}

bool StateContext::isEmptyMainState(void){
	if(current_state_.MainState)
		return false;
	return true;
}

unsigned char StateContext::getStateFlags(BaseState *_state)
{
	if(_state)
		return _state->getStateKind();
	else
		return NULL_STATE;
}

unsigned long long StateContext::getStateTransMask(BaseState *_state)
{
	if(_state)
		return _state->getStateTransMask();
	else
		return 0;
}

unsigned long long StateContext::getStateStateNum(BaseState *_state)
{
	if(_state)
		return _state->getStateNum();
	else
		return 0;
}


bool StateContext::setCurrentState(BaseState *_state)
{
  change_state_mutex.lock();

  BaseState *prevState = current_state_.MainState;

  if (isEmptyMainState() && _state )
  {
    current_state_.MainState = _state;
    std::cout << "Successed to set state \""
              << "NULL"
              << "\" to \"" << _state->getStateName() << "\" : Mask is [" << _state->getStateTransMask() << "/"
              << "NULL"
              << "-" << _state->getStateNum() << "]" << std::endl;
  }
  else
  {
    if (_state && (enableForceSetState || (_state->getStateTransMask() & prevState->getStateNum())))
    {
      switch (getStateFlags(_state))
      {
        case MAIN_STATE:
          current_state_.MainState = _state;
          current_state_.AccState = nullptr;
          current_state_.StrState = nullptr;
          current_state_.BehaviorState = nullptr;
          current_state_.PerceptionState = nullptr;
          current_state_.OtherState = nullptr;
          break;
        case ACC_STATE:
          current_state_.AccState = _state;
          break;
        case STR_STATE:
          current_state_.StrState = _state;
          break;
        case BEHAVIOR_STATE:
          current_state_.BehaviorState = _state;
          break;
        case PERCEPTION_STATE:
          current_state_.PerceptionState = _state;
          break;
        case OTHER_STATE:
          current_state_.OtherState = _state;
          break;
      }
      std::cout << "Successed to set state \"" << prevState->getStateName() << "\" to \"" << _state->getStateName()
                << "\" : Mask is [" << _state->getStateTransMask() << "/" << prevState->getStateNum() << "-"
                << _state->getStateNum() << "]" << std::endl;
    }
    else
    {
      std::cerr << "Failed to set state \"" << current_state_.MainState->getStateName() << "\" to \""
                << _state->getStateName() << "\" : Mask is [" << _state->getStateTransMask() << "/"
                << current_state_.MainState->getStateNum() << "-" << _state->getStateNum() << "]" << std::endl;
      prevState = nullptr;
      change_state_mutex.unlock();
      return false;
    }
  }
  change_state_mutex.unlock();
  return true;
}

#if 0
bool StateContext::setCurrentState(BaseState *_state)
{
  change_state_mutex.lock();

  BaseState *prevState = current_state_.MainState;

  if (!prevState)
  {
    current_state_.MainState = _state;
    std::cout << "Successed to set state \""
              << "NULL"
              << "\" to \"" << _state->getStateName() << "\" : Mask is [" << _state->getStateTransMask() << "/"
              << "NULL"
              << "-" << _state->getStateNum() << "]" << std::endl;
  }
  else
  {
    if (_state && (enableForceSetState || (_state->getStateTransMask() & prevState->getStateNum())))
    {
      switch (_state->getStateKind())
      {
        case MAIN_STATE:
          current_state_.MainState = _state;
          current_state_.AccState = nullptr;
          current_state_.StrState = nullptr;
          current_state_.BehaviorState = nullptr;
          current_state_.PerceptionState = nullptr;
          current_state_.OtherState = nullptr;
          break;
        case ACC_STATE:
          current_state_.AccState = _state;
          break;
        case STR_STATE:
          current_state_.StrState = _state;
          break;
        case BEHAVIOR_STATE:
          current_state_.BehaviorState = _state;
          break;
        case PERCEPTION_STATE:
          current_state_.PerceptionState = _state;
          break;
        case OTHER_STATE:
          current_state_.OtherState = _state;
          break;
      }
      std::cout << "Successed to set state \"" << prevState->getStateName() << "\" to \"" << _state->getStateName()
                << "\" : Mask is [" << _state->getStateTransMask() << "/" << prevState->getStateNum() << "-"
                << _state->getStateNum() << "]" << std::endl;
    }
    else
    {
      std::cerr << "Failed to set state \"" << current_state_.MainState->getStateName() << "\" to \""
                << _state->getStateName() << "\" : Mask is [" << _state->getStateTransMask() << "/"
                << current_state_.MainState->getStateNum() << "-" << _state->getStateNum() << "]" << std::endl;
      prevState = nullptr;
      change_state_mutex.unlock();
      return false;
    }
  }
  change_state_mutex.unlock();
  return true;
}
#endif

bool StateContext::setCurrentState(StateFlags flag)
{
  bool ret = this->setCurrentState(StateStores[flag]);
  return ret;
}

bool StateContext::setEnableForceSetState(bool force_flag)
{
  enableForceSetState = force_flag;
  return true;
}

std::string StateContext::getCurrentStateName(StateKinds en)
{
  for (auto &&p : HolderList)
  {
    if (*p && (*p)->getStateKind() == en)
	      return (*p)->getStateName();
  }
  return std::string("");
}
std::string StateContext::getCurrentStateName(void)
{
	return this->getCurrentStateName(MAIN_STATE);
}

BaseState *StateContext::getCurrentMainState(void)
{
  return current_state_.MainState;
}

BaseState *StateContext::getStateObject(unsigned long long _state_num)
{
  return StateStores[_state_num];
}

BaseState **StateContext::getCurrentStateHolderPtr(unsigned long long _state_num)
{
  BaseState **state_ptr;
  switch (getStateObject(_state_num)->getStateKind())
  {
    case MAIN_STATE:
      state_ptr = &current_state_.MainState;
      break;
    case ACC_STATE:
      state_ptr = &current_state_.AccState;
      break;
    case STR_STATE:
      state_ptr = &current_state_.StrState;
      break;
    case BEHAVIOR_STATE:
      state_ptr = &current_state_.BehaviorState;
      break;
    case PERCEPTION_STATE:
      state_ptr = &current_state_.PerceptionState;
      break;
    case OTHER_STATE:
      state_ptr = &current_state_.OtherState;
      break;
    default:
      state_ptr = nullptr;
      break;
  }
  return state_ptr;
}

bool StateContext::disableCurrentState(unsigned long long _state_num)
{
  BaseState **state_ptr = getCurrentStateHolderPtr(_state_num);
  if (state_ptr && this->isState((*state_ptr), _state_num))
  {
    *state_ptr = nullptr;
    return true;
  }
  else
  {
    return false;
  }
}

bool StateContext::isCurrentState(unsigned long long _state_num)
{
  BaseState **state_ptr = getCurrentStateHolderPtr(_state_num);
  return (*state_ptr) ? (*state_ptr)->getStateNum() & _state_num ? true : false : false;
}

bool StateContext::isState(BaseState *base, unsigned long long _state_num)
{
  return base ? base->getStateNum() & _state_num ? true : false : false;
}

bool StateContext::inState(unsigned long long _state_num)
{
  if (current_state_.MainState)
  {
    return ((current_state_.MainState->getStateNum() & _state_num) != 0) ? true : false;
  }
  else
  {
    return false;
  }
}

bool StateContext::handleIntersection(bool _hasIntersection, double _angle)
{
    /* deprecated */
	return false;
}

std::string StateContext::createStateMessageText(void)
{
  std::string ret;

  for (auto &&p : HolderList)
  {
    if (*p)
    {
      ret = ret + "\n" + (*p)->getStateName();
    }
  }
  return ret;
}

bool StateContext::handleTwistCmd(bool _hasTwistCmd)
{
  if (_hasTwistCmd)
    return this->setCurrentState(DRIVE_STATE);
  else
    return false;
}

void StateContext::stateDecider(void)
{
  // not running
}

void StateContext::InitContext(void)
{
  thr_state_dec = new std::thread(&StateContext::stateDecider, this);
  thr_state_dec->detach();
  this->setCurrentState(START_STATE);
  return;
}
bool StateContext::TFInitialized(void)
{
  return this->setCurrentState(INITIAL_STATE);
}
}
