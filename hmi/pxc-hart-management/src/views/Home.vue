<template>
  <div class="container">
    <HartDevice longTag="Dummy Device" hartRev="7"/>
    <LiveHartMuxGraphic />
    <HartMuxLiveList />
  </div>
</template>

<script>
// @ is an alias to /src
import HartDevice from '@/components/HartDevice.vue'
import HartMuxLiveList from '@/components/HartMuxLiveList.vue'
import LiveHartMuxGraphic from '@/components/live-graphics/LiveHartMuxGraphic.vue'

export default {
  name: 'home',
  components: {
    HartDevice,
    LiveHartMuxGraphic,
    HartMuxLiveList
  }
}
</script>
