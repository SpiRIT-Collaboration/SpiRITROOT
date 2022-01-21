void GetPhiEff(const std::vector<std::string>& input, const std::string& output, const std::string& outputBias, int nClus, double DPoca)
{
  STAnaParticleDB::EnableChargedParticles();
  STAnaParticleDB::EnablePions();
  STPhiEfficiencyTask::CreatePhiEffFromData(input, output, nClus, DPoca);
  STReactionPlaneTask::CreateBiasCorrection(input[input.size()/2], outputBias);
}
