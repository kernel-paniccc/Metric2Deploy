# Meric2Deploy
Full deployment cycle for the metrics application

## Fast start

### 1) k3s cluster deployment

```
cd Vagrant-k3s-cluster

vagrant up --provider=libvirt

ansible-playbook -i ansible/inventory.ini ansible/k3s-claster.yml

```