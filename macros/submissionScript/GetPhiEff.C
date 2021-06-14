void GetPhiEff(const std::vector<std::string>& input, const std::string& output, int nClus, double DPoca)
{
  STAnaParticleDB::EnableChargedParticles();
  STAnaParticleDB::EnablePions();
  STPhiEfficiencyTask::CreatePhiEffFromData(input, output, nClus, DPoca);
}
