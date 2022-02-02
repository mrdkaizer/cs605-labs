# Lab: Getting started with CloudLab

You will do your labs and assignments for CS 605 using [CloudLab](http://cloudlab.us/). CloudLab is a research facility which provides bare-metal access and control over a substantial set of computing, storage, and networking resources. If you haven’t worked in CloudLab before, you need to register a CloudLab account.

This lab walks you through the CloudLab registration process and shows you how to start an experiment in CloudLab.

Most importantly, it introduces our policies on using CloudLab that will be enforced throughout the semester.

## Register a CloudLab account

To register an account, please visit http://cloudlab.us and create an account using your University of Cyprus email address as login. Note that an SSH public key is required to access the nodes CloudLab assigns to you; if you are unfamiliar with creating and using ssh keypairs, we recommend taking a look at the first few steps in [GitHub’s guide to generating SSH keys](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent). (Obviously, the steps about how to upload the keypair into GitHub don’t apply to CloudLab.) For Windows Users, a good option for a Windows ssh client is to make use of [putty](https://www.putty.org/) and [generate and manage ssh keys with putty](https://devops.ionos.com/tutorials/use-ssh-keys-with-putty-on-windows/).

Click on Join Existing Project and enter ``UCY-CS499-DC`` as the project name. Then click on Submit Request. The project leader will approve your request. If you already have a CloudLab account, simply request to join the ``UCY-CS499-DC`` project.

## Start An Experiment

To start a new experiment, go to your CloudLab dashboard and click on the ``Experiments`` tab in the upper left corner, then select ``Start Experiment``. This will lead to the profile selection panel. Click on ``Change Profile``, and select a profile from the list. For example, if you choose the ``single-node-ubuntu18`` profile in the ``UCY-CS499-DC`` project you will be able to launch 1 machine with the Ubuntu 18 image. Select the profile and click on Next to move to the next panel. Here you should name your experiment with CloudLabLogin-ExperimentName. The purpose of doing this is to prevent everyone from picking random names and ending up confusing each other since everyone in the ``UCY-CS499-DC``  project can see a full list of experiments created. You also need to specify from which cluster you want to start your experiment. Each cluster has different hardwares. For more information on the hardware CloudLab provides, please refer to [this](http://docs.cloudlab.us/hardware.html). Once you select the cluster you can instantiate the experiment. Once the experiment is ready you will see the ssh login command. Try to login to the machine and check for the number of CPU cores available and memory available on the node.

## Policies on Using CloudLab Resources

Do not leave your CloudLab experiment instantiated unless you are using it! It is important to be a good citizen of CloudLab.

The nodes you receive from CloudLab are real hardware machines sitting in different clusters. Therefore, we ask you not to hold the nodes for too long. CloudLab gives users 16 hours to start with, and users can extend it for a longer time. Manage your time efficiently and only hold onto those nodes when you are working on the assignment. You should use a private git repository to manage your code, and you must terminate the nodes when you are not using them. If you do have a need to extend the nodes, do not extend them by more than 1 day. We will terminate any cluster running for more than 48 hours.

As a member of the ``UCY-CS499-DC`` project, you have permissions to access another member’s private user space. Stick to your own space and do not access others’ to peek at/copy/use their code, or intentionally/unintentionally overwriting files in others’ workspaces.

## References

1. [Gettting started. The CloudLab manual.](https://docs.cloudlab.us/getting-started.html)
2. [CloudLab users. The CloudLab manual.](https://docs.cloudlab.us/users.html)
