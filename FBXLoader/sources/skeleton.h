#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <hasenpfote/math/cmatrix4.h>

class SkeletonJoint
{
private:
    int parent_index;
    std::string name;
    hasenpfote::math::CMatrix4 inv_bindpose;
    hasenpfote::math::CMatrix4 localpose;
    hasenpfote::math::CMatrix4 globalpose;

public:
    void SetParentIndex(int index) { parent_index = index; }
    int GetParentIndex() const { return parent_index; }

    void SetName(const std::string name) { this->name = name; }
    std::string GetName() const { return name; }

    void SetInverseBindPose(const hasenpfote::math::CMatrix4& m) { inv_bindpose = m; }
    hasenpfote::math::CMatrix4 GetInverseBindPose() const { return inv_bindpose; }

    void SetLocalPose(const hasenpfote::math::CMatrix4& m) { localpose = m; }
    hasenpfote::math::CMatrix4 GetLocalPose() const { return localpose; }

    void SetGlobalPose(const hasenpfote::math::CMatrix4& m) { globalpose = m; }
    hasenpfote::math::CMatrix4 GetGlobalPose() const { return globalpose; }
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
    hasenpfote::math::CMatrix4 GetGlobalMatrix(std::string name) const;
    void SetJoint(const SkeletonJoint& joint);
    const std::vector<SkeletonJoint>& GetJoints() const { return joints; }

    void BuildByInvBindPose();
};