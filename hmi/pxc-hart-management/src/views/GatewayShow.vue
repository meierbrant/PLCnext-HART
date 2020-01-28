<template>
  <div class="container">
    <HartDevice longTag="Dummy Device" hartRev="7"/>
    <LiveHartMuxGraphic :gwIp=$route.params.ip />
    <HartMuxLiveList :gwIp=$route.params.ip />
  </div>
</template>

<script>
// @ is an alias to /src
import HartDevice from '@/components/HartDevice.vue'
import LiveHartMuxGraphic from '@/components/live-graphics/LiveHartMuxGraphic.vue'
import HartMuxLiveList from '@/components/HartMuxLiveList.vue'

export default {
  name: 'gateway-show',
  components: {
    HartDevice,
    LiveHartMuxGraphic,
    HartMuxLiveList
  }
}
</script>

<style lang="scss" scoped>
</style>