<template>
    <div class="circle-indicator" v-bind:class="{ online: status == 'online' }"
        v-b-popover.hover.top="'deviceData.name'"
        v-bind:title="'device.longTag'"></div>
</template>

<script lang="ts">
import { Vue, Component, Prop } from 'vue-property-decorator'
import { HartDeviceDto } from '../../types'

type status = 'online' | 'offline';

@Component
export default class DeviceBubble extends Vue {
    @Prop(Object) deviceData!: () => HartDeviceDto

    mounted () {
        
    }

    get status(): status {
        return this.deviceData !== undefined ? 'online' : 'offline'
    }

    get name(): string {
        return this.deviceData.name
    }
}
</script>