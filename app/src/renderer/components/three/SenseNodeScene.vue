<script setup lang="ts">
import { computed, onBeforeUnmount, onMounted, ref, watch } from 'vue'
import * as THREE from 'three'
import MetricGrid from '../panels/MetricGrid.vue'
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'
import { useSettingsStore } from '../../stores/settingsStore'
import type { SenseNodeFrame } from '../../../shared/types/sensenode'

const canvasHost = ref<HTMLDivElement | null>(null)
const device = useDeviceStore()
const settings = useSettingsStore()
const { t } = useI18n()

let renderer: THREE.WebGLRenderer | null = null
let scene: THREE.Scene | null = null
let camera: THREE.PerspectiveCamera | null = null
let node: THREE.Group | null = null
let grid: THREE.GridHelper | null = null
let keyLight: THREE.DirectionalLight | null = null
let fillLight: THREE.AmbientLight | null = null
let animationId = 0
let targetRotation = new THREE.Euler(0, 0, 0)
const fixedCameraPosition = new THREE.Vector3(0, 0.92, 4.75)
const fixedCameraTarget = new THREE.Vector3(0, 0.12, 0)

const metrics = computed(() => [
	{
		label: t('device.stableEvent'),
		value: device.currentFrame ? device.stableEvent : null,
	},
	{
		label: t('device.rawEvent'),
		value: device.currentFrame ? device.rawEvent : null,
	},
	{ label: t('scene.tofMm'), value: device.currentFrame?.tof_mm },
	{
		label: t('scene.confidence'),
		value: device.currentFrame?.confidence?.toFixed(2),
	},
])

onMounted(() => {
	if (!canvasHost.value) return
	scene = new THREE.Scene()
	camera = new THREE.PerspectiveCamera(32, 1, 0.1, 100)
	camera.position.copy(fixedCameraPosition)
	camera.lookAt(fixedCameraTarget)

	renderer = new THREE.WebGLRenderer({
		antialias: true,
		alpha: true,
		powerPreference: 'high-performance',
	})
	renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2))
	canvasHost.value.appendChild(renderer.domElement)

	keyLight = new THREE.DirectionalLight('#d6e4ff', 1.8)
	keyLight.position.set(2, 4, 3)
	fillLight = new THREE.AmbientLight('#9ba6b2', 1.2)
	scene.add(keyLight, fillLight)

	node = createNodeModel()
	node.position.y = 0.18
	scene.add(node)
	grid = createGrid()
	scene.add(grid)
	applySceneTheme(settings.themeMode)

	window.addEventListener('resize', resize)
	resize()
	animate()
})

watch(
	() => device.currentFrame,
	(frame) => {
		targetRotation = resolveTargetRotation(frame)
	},
)

watch(
	() => settings.themeMode,
	(themeMode) => applySceneTheme(themeMode),
)

onBeforeUnmount(() => {
	window.removeEventListener('resize', resize)
	cancelAnimationFrame(animationId)
	renderer?.dispose()
})

function animate(): void {
	animationId = requestAnimationFrame(animate)
	if (!renderer || !scene || !camera || !node) return

	node.rotation.x = THREE.MathUtils.lerp(
		node.rotation.x,
		targetRotation.x,
		0.24,
	)
	node.rotation.y = THREE.MathUtils.lerp(
		node.rotation.y,
		targetRotation.y,
		0.24,
	)
	node.rotation.z = THREE.MathUtils.lerp(
		node.rotation.z,
		targetRotation.z,
		0.24,
	)
	node.scale.setScalar(1)
	camera.position.copy(fixedCameraPosition)
	camera.lookAt(fixedCameraTarget)
	renderer.render(scene, camera)
}

function resize(): void {
	if (!canvasHost.value || !renderer || !camera) return
	const rect = canvasHost.value.getBoundingClientRect()
	renderer.setSize(rect.width, rect.height, false)
	camera.aspect = rect.width / rect.height
	camera.updateProjectionMatrix()
}

function createNodeModel(): THREE.Group {
	const group = new THREE.Group()
	const board = new THREE.Mesh(
		new THREE.BoxGeometry(2.9, 0.22, 0.92),
		new THREE.MeshStandardMaterial({
			color: '#17251f',
			metalness: 0.2,
			roughness: 0.55,
			emissive: '#0d1b16',
			emissiveIntensity: 0.25,
		}),
	)
	group.add(board)

	const shell = new THREE.LineSegments(
		new THREE.EdgesGeometry(new THREE.BoxGeometry(3.02, 0.25, 1.04)),
		new THREE.LineBasicMaterial({
			color: '#9ba6b2',
			transparent: true,
			opacity: 0.36,
		}),
	)
	group.add(shell)

	for (let index = 0; index < 4; index += 1) {
		const marker = new THREE.Mesh(
			new THREE.CylinderGeometry(0.065, 0.065, 0.025, 24),
			new THREE.MeshStandardMaterial({
				color: index === 0 ? '#b4c7b4' : '#4b5563',
			}),
		)
		marker.rotation.x = Math.PI / 2
		marker.position.set(-1.05 + index * 0.7, 0.14, -0.26 + (index % 2) * 0.5)
		group.add(marker)
	}

	return group
}

function createGrid(): THREE.LineSegments {
	const helper = new THREE.GridHelper(7, 14, '#94a3b8', '#cbd5e1')
	helper.position.y = -0.42
	return helper
}

function resolveTargetRotation(frame: SenseNodeFrame | null): THREE.Euler {
	if (!frame?.accel) {
		return new THREE.Euler(0, 0, 0)
	}

	const gyroMag = vectorMag(frame.gyro)
	const accelDelta = Math.abs(vectorMag(frame.accel) - 1)
	if (gyroMag < 0.55 && accelDelta < 0.24) {
		return new THREE.Euler(0, 0, 0)
	}

	const pitch = frame.pitch ?? Math.atan2(frame.accel.y, frame.accel.z)
	const roll =
		frame.roll ??
		Math.atan2(
			-frame.accel.x,
			Math.sqrt(frame.accel.y ** 2 + frame.accel.z ** 2),
		)
	const yaw = frame.yaw ?? targetRotation.y * 0.96
	return new THREE.Euler(clampAngle(pitch), clampAngle(yaw), clampAngle(roll))
}

function vectorMag(value?: { x: number; y: number; z: number }): number {
	if (!value) {
		return 0
	}
	return Math.sqrt(value.x * value.x + value.y * value.y + value.z * value.z)
}

function clampAngle(value: number): number {
	return THREE.MathUtils.clamp(value, -Math.PI * 0.7, Math.PI * 0.7)
}

function applySceneTheme(themeMode: 'dark' | 'light'): void {
	if (!scene || !keyLight || !fillLight || !grid) return

	if (themeMode === 'light') {
		scene.background = new THREE.Color('#f8fafc')
		keyLight.color.set('#1f2937')
		keyLight.intensity = 1.45
		fillLight.color.set('#94a3b8')
		fillLight.intensity = 1.65
		setGridColors(grid, '#94a3b8', '#dbe3ee')
		return
	}

	scene.background = new THREE.Color('#080A0F')
	keyLight.color.set('#d6e4ff')
	keyLight.intensity = 1.8
	fillLight.color.set('#9ba6b2')
	fillLight.intensity = 1.2
	setGridColors(grid, '#1f2937', '#111827')
}

function setGridColors(
	helper: THREE.GridHelper,
	center: string,
	gridColor: string,
): void {
	const material = helper.material
	if (Array.isArray(material)) {
		material[0]?.color.set(center)
		material[1]?.color.set(gridColor)
	}
}
</script>

<template>
	<div class="scene-layout">
		<div ref="canvasHost" class="scene-host panel" />
		<div v-if="!device.currentFrame" class="panel notice">
			{{ t('device.notConnected') }}
		</div>
		<MetricGrid :items="metrics" />
		<div class="debug panel mono">
			<span
				>{{ t('device.stabilizerState') }}:
				{{ device.stabilizerSnapshot.state }}</span
			>
			<span>{{
				device.currentFrame ? device.stabilizerSnapshot.reason : '-'
			}}</span>
			<span
				>{{ t('scene.lastTransition') }}
				{{
					new Date(
						device.stabilizerSnapshot.lastTransitionAt,
					).toLocaleTimeString()
				}}</span
			>
		</div>
	</div>
</template>

<style scoped>
.scene-layout {
	display: grid;
	grid-template-rows: 1fr auto auto;
	gap: 12px;
	width: 100%;
	height: 100%;
	min-height: 0;
}

.scene-host {
	min-height: 0;
	overflow: hidden;
	display: grid;
	place-items: center;
}

.debug {
	display: flex;
	gap: 16px;
	padding: 10px;
	color: var(--color-text-muted);
	font-size: 12px;
}

.notice {
	padding: 10px;
	color: var(--color-text-muted);
}
</style>
