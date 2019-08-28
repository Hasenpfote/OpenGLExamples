#include <cassert>
#include "skeleton.h"

SkeletonJoint* Skeleton::GetJoint(int index)
{
    assert((index >= 0) && (index < joints.size()));
    return &joints[index];
}

const SkeletonJoint* Skeleton::GetJoint(int index) const
{
    assert((index >= 0) && (index < joints.size()));
    return &joints[index];
}

SkeletonJoint* Skeleton::GetJoint(std::string name)
{
    decltype(map)::const_iterator it = map.find(name);
    if(it != map.cend())
        return &joints[it->second];
    return nullptr;
}

const SkeletonJoint* Skeleton::GetJoint(std::string name) const
{
    decltype(map)::const_iterator it = map.find(name);
    if(it != map.cend())
        return &joints[it->second];
    return nullptr;
}

int Skeleton::GetJointIndex(std::string name) const
{
    decltype(map)::const_iterator it = map.find(name);
    if(it != map.cend())
        return it->second;
    return -1;
}

void Skeleton::LocalToGlobal()
{
    joints[0].SetGlobalPose(joints[0].GetLocalPose());
    for(auto i = 1; i < joints.size(); i++)
    {
        auto& joint = joints[i];
        const auto& parent_joint = joints[joint.GetParentIndex()];
        joint.SetGlobalPose(parent_joint.GetGlobalPose() * joint.GetLocalPose());
    }
}

void Skeleton::GlobalToLocal()
{
    joints[0].SetLocalPose(joints[0].GetGlobalPose());
    for(auto i = 1; i < joints.size(); i++)
    {
        auto& joint = joints[i];
        const auto& parent_joint = joints[joint.GetParentIndex()];
        //joint.SetLocalPose(hasenpfote::math::CMatrix4::InverseAffineTransformation(parent_joint.GetGlobalPose()) * joint.GetGlobalPose());
        joint.SetLocalPose(glm::inverse(parent_joint.GetGlobalPose()) * joint.GetGlobalPose());
    }
}

glm::mat4 Skeleton::GetGlobalMatrix(std::string name) const
{
    auto result = glm::mat4(1.0f);
    auto joint = GetJoint(name);
    if(!joint)
        return result;
    result = joint->GetLocalPose();
    auto pid = joint->GetParentIndex();
    while(pid >= 0)
    {
        const auto& parent_joint = joints[pid];
        result = parent_joint.GetLocalPose() * result;
        pid = parent_joint.GetParentIndex();
    }
    return result;
}

void Skeleton::SetJoint(const SkeletonJoint& joint)
{
    const auto index = static_cast<int>(joints.size());
    assert(index > joint.GetParentIndex());
    joints.push_back(joint);
    map.insert(std::make_pair(joint.GetName(), index));
}

void Skeleton::BuildByInvBindPose()
{
    glm::mat4 bindpose;
    for(auto& joint : joints)
    {
        //bindpose = CMatrix4::InverseAffineTransformation(joint.GetInverseBindPose());
        bindpose = glm::inverse(joint.GetInverseBindPose());
        joint.SetGlobalPose(bindpose);
    }
    GlobalToLocal();
}
