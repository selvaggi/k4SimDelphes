#ifndef DELPHESEDM4HEP_DELPHESEDM4HEP_CONVERTER_H__
#define DELPHESEDM4HEP_DELPHESEDM4HEP_CONVERTER_H__

// podio
#include "podio/EventStore.h"
#include "podio/ROOTWriter.h"
#include "podio/CollectionBase.h"

// edm4hep
#include "edm4hep/MCParticleConst.h"
#include "edm4hep/ReconstructedParticle.h"

// ROOT
#include "TTree.h"
#include "TClonesArray.h"

#include <vector>
#include <string>
#include <string_view>
#include <array>
#include <unordered_map>

// Delphes output classes
class Muon;
class Electron;

namespace k4simdelphes {

/**
 * Classes that will be stored as reconstructed particle with an attached track
 */
constexpr std::array<std::string_view, 1> RECO_TRACK_OUTPUT = {"Track"};

/**
 * Classes that will be stored as reconstructed particle with an attached cluster
 */
constexpr std::array<std::string_view, 1> RECO_CLUSTER_OUTPUT = {"Tower"};


/**
 * Order in which the different delphes output classes will be processed.
 * Everything not defined here will not be processed.
 *
 * NOTE: not a configuration parameter. this has to be done in this order to
 * ensure that products required by later stages are producd early enough
 */
constexpr std::array<std::string_view, 9> PROCESSING_ORDER = {
  "GenParticle",
  "Track",
  "Tower",
  "Jet",
  "Muon",
  "Electron",
  "Photon",
  "MissingET",
  "SclalarHT"
};


struct BranchSettings {
  std::string input;
  std::string name;
  std::string className;
};

class OutputSettings;

class DelphesEDM4HepConverter {
public:
  DelphesEDM4HepConverter(const std::vector<BranchSettings>& branches,
                          OutputSettings const& outputSettings, double magFieldBz);

  void process(TTree* delphesTree);

  inline std::unordered_map<std::string_view, podio::CollectionBase*> getCollections() { return m_collections; }

private:

  void processParticles(const TClonesArray* delphesCollection, std::string_view const branch);
  void processTracks(const TClonesArray* delphesCollection, std::string_view const branch);
  void processClusters(const TClonesArray* delphesCollection, std::string_view const branch);
  void processJets(const TClonesArray* delphesCollection, std::string_view const branch);
  void processPhotons(const TClonesArray* delphesCollection, std::string_view const branch);

  void processMissingET(const TClonesArray* delphesCollection, std::string_view const branch);
  void processScalarHT(const TClonesArray* delphesCollection, std::string_view const branch);

  void processMuons(const TClonesArray* delphesCollection, std::string_view const branch) {
    processMuonsElectrons<Muon>(delphesCollection, branch, "muon");
  }
  void processElectrons(const TClonesArray* delphesCollection, std::string_view const branch) {
    processMuonsElectrons<Electron>(delphesCollection, branch, "electron");
  }

  // Muon and Electron are almost the same in Delphes, but the class layout is
  // not exactly the same so we have to inject the desired type via a template here
  template<typename DelphesT>
  void processMuonsElectrons(const TClonesArray* delphesCollection, std::string_view const branch, std::string_view type);

  void registerGlobalCollections();

  template<typename CollectionT>
  void createCollection(std::string_view const name);

  using ProcessFunction = void (DelphesEDM4HepConverter::*)(const TClonesArray*, std::string_view const);

  std::vector<BranchSettings> m_branches;
  std::unordered_map<std::string_view, podio::CollectionBase*> m_collections;
  std::unordered_map<std::string_view, ProcessFunction> m_processFunctions;

  double m_magneticFieldBz; // necessary for determining track parameters

  std::string m_recoCollName;
  std::string m_particleIDName;
  std::string m_mcRecoAssocCollName;

  // map from UniqueIDs (delphes generated particles) to MCParticles
  std::unordered_map<UInt_t, edm4hep::ConstMCParticle> m_genParticleIds;
  // map from UniqueIDs (delphes generated particles) to (possibly multiple)
  // ReconstructedParticles
  std::unordered_multimap<UInt_t, edm4hep::ReconstructedParticle> m_recoParticleGenIds;
};

template<typename CollectionT>
void DelphesEDM4HepConverter::createCollection(std::string_view name) {
  std::string nameStr(name);
  CollectionT* col = new CollectionT();
  m_collections.emplace(name, col);
}

} //namespace k4simdelphes

#endif