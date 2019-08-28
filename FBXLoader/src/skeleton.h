#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

class SkeletonJoint
{
private:
    int parent_index;
    std::string name;
    glm::mat4 inv_bindpose;
    glm::mat4 localpose;
    glm::mat4 globalpose;

public:
    void SetParentIndex(int index){ parent_index = index; }
    int GetParentIndex() const { return parent_index; }

    void SetName(const std::string name){ this->name = name; }
    std::string GetName() const { return name; }

    void SetInverseBindPose(const glm::mat4& m){ inv_bindpose = m; }
    glm::mat4 GetInverseBindPose() const { return inv_bindpose; }

    void SetLocalPose(const glm::mat4& m){ localpose = m; }
    glm::mat4 GetLocalPose() const { return localpose; }

    void SetGlobalPose(const glm::mat4& m){ globalpose = m; }
    glm::mat4 GetGlobalPose() const { return globalpose; }
};

class Skeleton
{
private:
    std::vector<SkeletonJoint> joints;
    std::unordered_map<std::string, int> map;

public:
    SkeletonJoint* GetJoint(int index);
    const SkeletonJoint* GetJoint(int index) const;
    SkeletonJoint* GetJoint(std::string name);
    const SkeletonJoint* GetJoint(std::string name) const;
    int GetJointIndex(std::string name) const;
    void LocalToGlobal();
    void GlobalToLocal();
    glm::mat4 GetGlobalMatrix(std::string name) const;
    void SetJoint(const SkeletonJoint& joint);
    const std::vector<SkeletonJoint>& GetJoints() const { return joints; }

    void BuildByInvBindPose();
};