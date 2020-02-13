<template>
  <div class="container">
    <LiveHartMuxGraphic :gwSN=$route.params.serialNo />
    <HartMuxLiveList :gwSN=$route.params.serialNo />
  </div>
</template>

<script>
// @ is an alias to /src
import LiveHartMuxGraphic from '@/components/live-graphics/LiveHartMuxGraphic.vue'
import HartMuxLiveList from '@/components/HartMuxLiveList.vue'

export default {
  name: 'gateway-show',
  components: {
    LiveHartMuxGraphic,
    HartMuxLiveList
  }
}
</script>

<style lang="scss" scoped>
</style>