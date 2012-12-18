#include "URGScanner.hpp"

#include <cmath>

namespace URGController
{

    URGRules::URGRules()
        :
        info()
    {
        info.name_ = "None";
        info.type_ = NONE;
        info.steps_ = info.beginStep_ = info.endStep_ = 0;
        info.resol_ = info.beginDeg_ = 0.0;
    }

    std::string URGRules::getName() const { return info.name_;      }
    URGRules::ScannerType URGRules::getType() const { return info.type_; }
    int    URGRules::getSteps()     const { return info.steps_;     }
    int    URGRules::getBeginStep() const { return info.beginStep_; }
    int    URGRules::getEndStep()   const { return info.endStep_;   }
    double URGRules::getResol()     const { return info.resol_;     }
    double URGRules::getBeginDeg()  const { return info.beginDeg_;  }

    void URGRules::setName(std::string name) {
        info.name_ = name;
    }
    void URGRules::setType(ScannerType type) {
        info.type_ = type;
    }
    void URGRules::setSteps(int beginStep, int endStep) {
        info.beginStep_ = beginStep;
        info.endStep_   = endStep;
        info.steps_ = (info.endStep_ - info.beginStep_) + 1;
    }
    void URGRules::setResol(double resol)       { info.resol_    = resol;    }
    void URGRules::setBeginDeg(double beginDeg) { info.beginDeg_ = beginDeg; }

    void setURGRules(URGRules &r, const URGRules::ScannerInfo_t &si)
    {
        r.setName(si.name_);
        r.setType(si.type_);
        r.setSteps(si.beginStep_, si.endStep_);
        r.setResol(si.resol_);
        r.setBeginDeg(si.beginDeg_);
    }

}
