#ifndef OGTA_LOCAL_PLAYER_H
#define OGTA_LOCAL_PLAYER_H
#include "game_objects.h"
#include "entity_controller.h"
#include "id_sys.h"
#include "key_handler.h"
#include "m_exceptions.h"

namespace OpenGTA {

  class LocalPlayer : public Util::KeyHandler {
    public:
      LocalPlayer() {
        reset();
      }
      ~LocalPlayer() override = default;

      LocalPlayer(const LocalPlayer& copy) = delete;
      LocalPlayer& operator=(const LocalPlayer& copy) = delete;
      LocalPlayer(LocalPlayer&& move) = delete;
      LocalPlayer& operator=(LocalPlayer&& move) = delete;

      void reset() {
        playerId = TypeIdBlackBox::Instance().getPlayerId();
        cash = 0;
        wantedLevel = 0;
        numLives = 0;
        pc_ptr = nullptr;
      }
      PedController & getCtrl() {
        if (pc_ptr == nullptr)
          throw Util::NotSupported("Player is not available");
        return *pc_ptr;
      }
      void setCtrl(PedController & pc) {
        pc_ptr = &pc;
      }
      void giveLives(uint16_t k) {
        numLives += k;
      }
      Pedestrian & getPed() const;
      [[maybe_unused]] [[nodiscard]] int32_t  getNumLives() const { return numLives; }
      [[nodiscard]] int32_t  getWantedLevel() const { return wantedLevel; }
      [[nodiscard]] uint32_t getCash() const { return cash; }
      bool up(const uint32_t & key) override;
      bool down(const uint32_t & key) override;
      [[nodiscard]] uint32_t getId() const { return playerId; }
      void addCash(uint32_t v) { cash += v; }
      void setWanted(int32_t v) { wantedLevel = v; }
      void addWanted(uint32_t v) { wantedLevel += v; if (wantedLevel > 5) wantedLevel = 5; }
    private:
      uint32_t playerId{};
      uint32_t cash{};
      int32_t  wantedLevel{};
      int32_t  numLives{};
      PedController * pc_ptr{};
  };
}

#endif
